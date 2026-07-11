use probe_rs::flashing::{DownloadOptions, ElfOptions, Format};
use probe_rs::probe::stlink::StLinkFactory;
use probe_rs::probe::{DebugProbeInfo, DebugProbeSelector, Probe, ProbeFactory};
use probe_rs::{Permissions, Session};
use std::collections::HashMap;
use std::thread;
use std::time::Duration;

/// Worker thread -> main thread: worker-level events
pub(crate) enum WorkerEvent {
    Status(String, String),
    Died(String),
}

/// Control commands the manager sends to a worker
pub(crate) enum WorkerCmd {
    Kill,   // SN is fully gone from the hardware list; force the worker to stop
    WakeUp, // USB changed; interrupt the wait and re-check own hardware state right away
}

/// Create and start a worker (skip if this sn already has one).
/// Resolve the DebugProbeInfo from the selector, then hand it to worker_loop.
pub(crate) fn spawn_worker(
    sn: String,
    selector: DebugProbeSelector,
    active_workers: &mut HashMap<String, crossbeam_channel::Sender<WorkerCmd>>,
    worker_tx: &crossbeam_channel::Sender<WorkerEvent>,
) {
    if active_workers.contains_key(&sn) {
        return;
    }
    // selector -> the matching probe; skip building for now if not found (maybe not enumerated yet).
    // Only list ST-Links, no full Lister driver scan.
    let stlink_factory: StLinkFactory = StLinkFactory {};
    let Some(probe_info) = stlink_factory
        .list_probes_filtered(Some(&selector))
        .into_iter()
        .next()
    else {
        return;
    };
    let (cmd_tx, cmd_rx) = crossbeam_channel::bounded(1);
    active_workers.insert(sn.clone(), cmd_tx);
    let worker_tx = worker_tx.clone();
    thread::spawn(move || worker_loop(sn, probe_info, worker_tx, cmd_rx));
}

/// On worker exit (any return or panic), drop sends one Died; a single death-notification path.
struct DiedGuard {
    sn: String,
    tx: crossbeam_channel::Sender<WorkerEvent>,
}

impl Drop for DiedGuard {
    fn drop(&mut self) {
        // The main thread may have closed the channel already; ignore send failure.
        let _ = self.tx.send(WorkerEvent::Died(self.sn.clone()));
    }
}

impl DiedGuard {
    /// Send a status to the main thread (ignore if the main side is closed).
    fn status(&self, msg: &str) {
        let _ = self
            .tx
            .send(WorkerEvent::Status(self.sn.clone(), msg.to_string()));
    }
}

// -----------------------------------------------------------------
// Worker thread: instant WakeUp-response logic
// -----------------------------------------------------------------
fn worker_loop(
    sn: String,
    probe_info: DebugProbeInfo,
    tx: crossbeam_channel::Sender<WorkerEvent>,
    cmd_rx: crossbeam_channel::Receiver<WorkerCmd>,
) {
    let target_mcu = "STM32F103CBT6";
    // Whichever path it exits by (open fail, voltage error, Kill, panic), the guard sends one Died on drop.
    let guard = DiedGuard { sn, tx };

    loop {
        guard.status("loop 開始，檢查硬體狀態...");
        // Open the ST-Link; if it won't open, the ST-Link is gone, so exit.
        let Some(mut probe) = open_probe(&probe_info) else {
            return;
        };

        // 1. Wait until the board is stably present (N consecutive), avoiding flashing on flaky contact.
        match wait_until_stable(&mut probe, &cmd_rx, true) {
            Signal::Kill => return,
            Signal::Stable => {}
        }

        // 2. attach + flash; on flash failure loop back and retry (session drops on continue).
        let Ok(mut session) = probe.attach(target_mcu, Permissions::default()) else {
            continue; // just confirmed present but attach failed (transient); retry
        };
        if !flash(&guard, &mut session) {
            continue;
        }

        // 3. Start the application
        start_app(&guard, &mut session);
        drop(session); // explicitly release the probe so the next step can reopen the ST-Link

        // 4. Wait until the board is stably removed (reopen a probe to poll).
        let Some(mut probe) = open_probe(&probe_info) else {
            return;
        };
        match wait_until_stable(&mut probe, &cmd_rx, false) {
            Signal::Kill => return,
            Signal::Stable => {}
        }
    }
}

/// Open the probe and confirm target voltage; either failing returns None (the worker should exit).
fn open_probe(probe_info: &DebugProbeInfo) -> Option<Probe> {
    let mut probe = probe_info.open().ok()?;
    probe.get_target_voltage().ok()?;
    Some(probe)
}

/// Firmware to flash, embedded into the binary (no runtime file dependency).
const FIRMWARE: &[u8] = include_bytes!("../../../fw/V1.1/fw.elf");
// const FIRMWARE: &[u8] = include_bytes!("../../../fw/V2.2 RELEASE/fw.elf");

/// Flashing flow after a successful attach; borrows session (no consume), returns true on success.
fn flash(guard: &DiedGuard, session: &mut Session) -> bool {
    guard.status(&format!("🎉 開始燒錄 {}...", session.target().name));

    // Build a flash loader from the embedded ELF (Cursor makes &[u8] Read + Seek).
    let mut loader = session.target().flash_loader();
    let mut image = std::io::Cursor::new(FIRMWARE);
    if let Err(e) = loader.load_image(
        session,
        &mut image,
        Format::Elf(ElfOptions::default()),
        None,
    ) {
        guard.status(&format!("❌ 載入韌體失敗：{e}"));
        return false;
    }
    if let Err(e) = loader.commit(session, DownloadOptions::default()) {
        guard.status(&format!("❌ 燒錄失敗：{e}"));
        return false;
    }

    guard.status("✅ 燒錄完成");
    true
}

/// Start the application after flashing: reset then let the core run, same as STM32_Programmer_CLI -s 0x08000000.
fn start_app(guard: &DiedGuard, session: &mut Session) {
    let mut core = match session.core(0) {
        Ok(core) => core,
        Err(e) => {
            guard.status(&format!("❌ 啟動失敗（取 core）：{e}"));
            return;
        }
    };
    if let Err(e) = core.reset() {
        guard.status(&format!("❌ 啟動失敗（reset）：{e}"));
        return;
    }
    guard.status("🚀 已啟動新韌體，請拔除板子");
}

/// How many consecutive identical readings count as "stable". Flaky cable contact makes the state
/// flip back and forth, so wait until it settles before moving to the next step.
const STABLE_COUNT: u32 = 5;
/// Interval between two polls.
const POLL_INTERVAL: Duration = Duration::from_millis(300);

/// Result of wait_until_stable
enum Signal {
    Stable, // reached stable; proceed
    Kill,   // stop the worker
}

/// Whether the board is present: attach_to_unspecified Ok = present. Always detach afterward, don't hold the debug port.
fn board_present(probe: &mut Probe) -> bool {
    let present = probe.attach_to_unspecified().is_ok();
    let _ = probe.detach();
    present
}

/// Poll until the board state stably reaches want_present (STABLE_COUNT consecutive equal readings), accepting Kill meanwhile.
/// Shared by connect and remove: want_present=true waits for stable presence, false for stable removal.
/// If the reading flips mid-way, reset the counter to avoid false triggers from flaky contact.
/// Returns Stable when settled (proceed), Kill when a Kill is received.
fn wait_until_stable(
    probe: &mut Probe,
    cmd_rx: &crossbeam_channel::Receiver<WorkerCmd>,
    want_present: bool,
) -> Signal {
    let mut stable = 0;
    loop {
        if board_present(probe) == want_present {
            stable += 1;
            if stable >= STABLE_COUNT {
                return Signal::Stable;
            }
        } else {
            stable = 0; // flipped to the other state; recount
        }
        match cmd_rx.recv_timeout(POLL_INTERVAL) {
            Ok(WorkerCmd::Kill) => return Signal::Kill, // full USB removal is signaled by the main loop
            Ok(WorkerCmd::WakeUp) | Err(_) => continue,
        }
    }
}
