use probe_rs::flashing::{DownloadOptions, ElfOptions, Format};
use probe_rs::probe::stlink::StLinkFactory;
use probe_rs::probe::{DebugProbeInfo, DebugProbeSelector, Probe, ProbeFactory};
use probe_rs::{Permissions, Session};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use crate::firmware::FwRegistry;
use crate::inject::inject;

/// Worker thread -> main thread: worker-level events
pub(crate) enum WorkerEvent {
    /// A status update (sn, phase, level, message). `phase` is the stage it happened in, `level`
    /// sets the color, `message` is free-form.
    Status(String, Phase, Level, String),
    /// The worker exited (sn, phase it died in, level, reason); the manager decides what to show next.
    Died(String, Phase, Level, String),
}

/// The stage a line is in: worker stages plus manager-side stages (no worker thread).
/// Errors and deaths carry their phase, so you can tell where they occurred.
#[derive(Clone, Copy)]
pub(crate) enum Phase {
    ProbeConnected, // manager: ST-Link present on USB, worker not (yet) running
    ProbeOpening,   // worker: opening the ST-Link, reading target voltage
    WaitingBoard,   // worker: waiting for the board to be stably inserted
    Flashing,       // worker: flashing firmware
    Booting,        // worker: resetting and running the new firmware
    Completed,      // worker: flashed and started; waiting for the board to be removed
    WaitingRestart, // manager: worker died with the device still present, waiting to respawn
}

impl std::fmt::Display for Phase {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let label = match self {
            Phase::ProbeConnected => "PROBE_CONN",
            Phase::ProbeOpening => "PROBE_OPEN",
            Phase::WaitingBoard => "WAIT_BOARD",
            Phase::Flashing => "FLASHING",
            Phase::Booting => "BOOTING",
            Phase::Completed => "DONE",
            Phase::WaitingRestart => "WAIT_RESTART",
        };
        f.write_str(label)
    }
}

/// Severity of a status message; the UI maps it to a color. Semantic, not a color, so senders
/// stay presentation-agnostic.
#[derive(Clone, Copy)]
pub(crate) enum Level {
    Info,     // neutral / in progress
    Progress, // actively doing work (e.g. flashing)
    Success,  // a step finished well
    Warn,     // needs attention (e.g. waiting for a board)
    Error,    // something failed
}

/// Why the worker loop ended. Carried out via `?`, so every exit must produce a reason; the guard
/// reports it on drop.
pub(crate) struct Death {
    level: Level,
    reason: String,
}

impl Death {
    fn error(reason: impl Into<String>) -> Self {
        Death {
            level: Level::Error,
            reason: reason.into(),
        }
    }
    fn killed() -> Self {
        Death {
            level: Level::Info,
            reason: "收到結束指令".into(),
        }
    }
}

/// Turn any error into an Error-level `Death` with a reason, keeping the original as detail.
/// Like anyhow's `.context`, but yields our `Death`: `probe.open().or_death("...")?`.
trait OrDeath<T> {
    fn or_death(self, reason: &str) -> Result<T, Death>;
}

impl<T, E: std::fmt::Display> OrDeath<T> for Result<T, E> {
    fn or_death(self, reason: &str) -> Result<T, Death> {
        self.map_err(|e| Death::error(format!("{reason}: {e}")))
    }
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
    registry: Arc<Mutex<FwRegistry>>,
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
    thread::spawn(move || {
        // Guard drops on this thread, sending exactly one Died however worker_loop ends (`?` Death
        // or panic). Borrow it (don't move) so we can record the Death after worker_loop returns.
        let mut guard = DiedGuard::new(sn, worker_tx);
        if let Err(death) = worker_loop(&mut guard, probe_info, cmd_rx, &registry) {
            guard.set_death(death.level, death.reason);
        }
    });
}

/// On worker exit (any Death or panic), drop sends one Died; a single death-notification path.
/// Holds the current phase so status/death carry the stage they hit. Access is sequential on this
/// thread, so plain fields + `&mut` suffice (no Cell).
struct DiedGuard {
    sn: String,
    tx: crossbeam_channel::Sender<WorkerEvent>,
    phase: Phase,
    death: Option<(Level, String)>,
}

impl DiedGuard {
    fn new(sn: String, tx: crossbeam_channel::Sender<WorkerEvent>) -> Self {
        DiedGuard {
            sn,
            tx,
            phase: Phase::ProbeOpening,
            death: None,
        }
    }
    /// Move to a new stage; later status and death inherit it.
    fn enter(&mut self, phase: Phase) {
        self.phase = phase;
    }
    /// Report status at the given level, tagged with the current phase (ignore if the main side is closed).
    fn status(&self, level: Level, msg: impl Into<String>) {
        let _ = self.tx.send(WorkerEvent::Status(
            self.sn.clone(),
            self.phase,
            level,
            msg.into(),
        ));
    }
    /// Record why the loop ended, so drop sends it (instead of the panic/unknown fallback).
    fn set_death(&mut self, level: Level, reason: String) {
        self.death = Some((level, reason));
    }
}

impl Drop for DiedGuard {
    fn drop(&mut self) {
        // Use the recorded reason; if none was set the loop didn't end via `?`, so it panicked.
        let (level, reason) = self.death.take().unwrap_or_else(|| {
            if std::thread::panicking() {
                (Level::Error, "worker panic，非預期結束".into())
            } else {
                (Level::Warn, "worker 結束（原因不明）".into())
            }
        });
        // The main thread may have closed the channel already; ignore send failure.
        let _ = self.tx.send(WorkerEvent::Died(
            self.sn.clone(),
            self.phase,
            level,
            reason,
        ));
    }
}

// -----------------------------------------------------------------
// Worker thread: instant WakeUp-response logic
// -----------------------------------------------------------------
fn worker_loop(
    guard: &mut DiedGuard,
    probe_info: DebugProbeInfo,
    cmd_rx: crossbeam_channel::Receiver<WorkerCmd>,
    registry: &Mutex<FwRegistry>,
) -> Result<(), Death> {
    let target_mcu = "STM32F103CBT6";

    // The loop never returns Ok; every exit is a `?` producing a Death, so the reason can't be
    // forgotten (open_probe failure and Kill both surface here).
    loop {
        guard.enter(Phase::ProbeOpening);
        guard.status(Level::Info, "開啟並檢測 ST-Link...");
        let mut probe = open_probe(&probe_info)?;

        // 1. Wait until the board is stably present (N consecutive), avoiding flashing on flaky contact.
        guard.enter(Phase::WaitingBoard);
        guard.status(Level::Warn, "等待插入板子...");
        wait_until_stable(&mut probe, &cmd_rx, true)?;

        // 2. attach + flash; on flash failure loop back and retry (session drops on continue).
        let Ok(mut session) = probe.attach(target_mcu, Permissions::default()) else {
            continue; // just confirmed present but attach failed (transient); retry
        };
        guard.enter(Phase::Flashing);
        if !flash(guard, &mut session, registry) {
            continue;
        }

        // 3. Start the application
        start_app(guard, &mut session);
        drop(session); // explicitly release the probe so the next step can reopen the ST-Link

        // 4. Wait until the board is stably removed (reopen a probe to poll).
        let mut probe = open_probe(&probe_info)?;
        wait_until_stable(&mut probe, &cmd_rx, false)?;
    }
}

/// Open the probe and read target voltage; either failing is a Death (worker should exit) with a
/// specific reason.
fn open_probe(probe_info: &DebugProbeInfo) -> Result<Probe, Death> {
    let mut probe = probe_info.open().or_death("ST-Link 開啟失敗")?;
    probe.get_target_voltage().or_death("讀取目標電壓失敗")?;
    Ok(probe)
}

/// Flashing flow after a successful attach; borrows session (no consume), returns true on success.
/// The caller has entered Phase::Flashing, so errors here are tagged with it.
fn flash(guard: &mut DiedGuard, session: &mut Session, registry: &Mutex<FwRegistry>) -> bool {
    // Read the firmware selected for the active year; clone the source out and drop the lock
    // before any file IO or flashing.
    let (label, source) = {
        let reg = registry.lock().unwrap();
        let choice = reg.active_choice();
        (choice.label.clone(), choice.source.clone())
    };

    guard.status(
        Level::Progress,
        format!("開始燒錄 {}（{label}）...", session.target().name),
    );

    // Embedded returns instantly; a runtime File is re-read here, so a missing/unreadable file
    // fails just this board (retry) instead of killing the worker.
    let bytes = match source.load() {
        Ok(bytes) => bytes,
        Err(e) => {
            guard.status(Level::Error, format!("讀取韌體失敗：{e}"));
            return false;
        }
    };

    let injected = inject(&bytes);
    // Warn on any missing placeholder; silent on full success.
    let missing: Vec<&str> = injected
        .replacements
        .iter()
        .filter(|(_, count)| *count == 0)
        .map(|(name, _)| *name)
        .collect();
    if !missing.is_empty() {
        guard.status(
            Level::Warn,
            format!("韌體 placeholder 取代失敗: {}", missing.join(", ")),
        );
    }

    // Build a flash loader from the injected image (Cursor makes &[u8] Read + Seek).
    let mut loader = session.target().flash_loader();
    let mut image = std::io::Cursor::new(injected.image.as_slice());
    if let Err(e) = loader.load_image(
        session,
        &mut image,
        Format::Elf(ElfOptions::default()),
        None,
    ) {
        guard.status(Level::Error, format!("載入韌體失敗：{e}"));
        return false;
    }
    // verify: read the flash back after writing and confirm it matches. DownloadOptions is
    // non_exhaustive, so set the field on default() instead of a struct literal.
    let mut options = DownloadOptions::default();
    options.verify = true;
    if let Err(e) = loader.commit(session, options) {
        guard.status(Level::Error, format!("燒錄或驗證失敗：{e}"));
        return false;
    }

    guard.status(Level::Success, "燒錄與驗證完成");
    true
}

/// Start the application after flashing: reset then let the core run, same as STM32_Programmer_CLI -s 0x08000000.
fn start_app(guard: &mut DiedGuard, session: &mut Session) {
    guard.enter(Phase::Booting);
    let mut core = match session.core(0) {
        Ok(core) => core,
        Err(e) => {
            guard.status(Level::Error, format!("啟動失敗（取 core）：{e}"));
            return;
        }
    };
    if let Err(e) = core.reset() {
        guard.status(Level::Error, format!("啟動失敗（reset）：{e}"));
        return;
    }
    guard.enter(Phase::Completed);
    guard.status(Level::Success, "已啟動新韌體，請移除板子");
}

/// How many consecutive identical readings count as "stable". Flaky cable contact makes the state
/// flip back and forth, so wait until it settles before moving to the next step.
const STABLE_COUNT: u32 = 5;
/// Interval between two polls.
const POLL_INTERVAL: Duration = Duration::from_millis(300);

/// Whether the board is present: attach_to_unspecified Ok = present. Always detach afterward, don't hold the debug port.
fn board_present(probe: &mut Probe) -> bool {
    let present = probe.attach_to_unspecified().is_ok();
    let _ = probe.detach();
    present
}

/// Poll until the board state stably reaches want_present (STABLE_COUNT consecutive equal readings), accepting Kill meanwhile.
/// Shared by connect and remove: want_present=true waits for stable presence, false for stable removal.
/// If the reading flips mid-way, reset the counter to avoid false triggers from flaky contact.
/// Returns Ok when settled (proceed), Err(Death) when a Kill is received.
fn wait_until_stable(
    probe: &mut Probe,
    cmd_rx: &crossbeam_channel::Receiver<WorkerCmd>,
    want_present: bool,
) -> Result<(), Death> {
    let mut stable = 0;
    loop {
        if board_present(probe) == want_present {
            stable += 1;
            if stable >= STABLE_COUNT {
                return Ok(());
            }
        } else {
            stable = 0; // flipped to the other state; recount
        }
        match cmd_rx.recv_timeout(POLL_INTERVAL) {
            Ok(WorkerCmd::Kill) => return Err(Death::killed()), // full USB removal is signaled by the main loop
            Ok(WorkerCmd::WakeUp) | Err(_) => continue,
        }
    }
}
