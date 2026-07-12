use futures::executor::block_on_stream;
use nusb::watch_devices;
use nusb::{DeviceId, MaybeFuture, hotplug::HotplugEvent};
use probe_rs::probe::DebugProbeSelector;
use ratatui::crossterm::event::{self, Event, KeyEventKind};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

mod device;
mod ecc;
mod firmware;
mod inject;
mod stlink_tools;
mod ui;
mod worker;

use firmware::FwRegistry;

use device::{DeviceEvent, TrackedDevice, on_connected, on_disconnected};
use ui::Ui;
use worker::{Phase, WorkerCmd, WorkerEvent, spawn_worker};

/// How long to wait before respawning a worker whose device is still present (avoids a die->respawn tight loop).
const RESPAWN_BACKOFF: Duration = Duration::from_millis(500);

fn main() {
    let (dev_tx, dev_rx) = crossbeam_channel::unbounded::<DeviceEvent>();
    let (worker_tx, worker_rx) = crossbeam_channel::unbounded::<WorkerEvent>();
    // Backoff respawn requests: after Died with the device still present, send the sn back later.
    let (respawn_tx, respawn_rx) = crossbeam_channel::unbounded::<String>();

    let mut active_workers: HashMap<String, crossbeam_channel::Sender<WorkerCmd>> = HashMap::new();

    // Firmware selection, shared with every worker (read at flash time) and the UI (rendered + mutated by keys).
    let registry = Arc::new(Mutex::new(FwRegistry::default()));

    // Device state, centralized in the main loop so Died can look it up.
    // serial -> its dev ids (Vec only in case serials collide); None = no-serial devices.
    let mut serial_to_ids: HashMap<Option<String>, Vec<DeviceId>> = HashMap::new();
    // id -> device: Disconnected only gives an id, so this reverse-maps to the serial (and keeps DeviceInfo).
    let mut id_to_device: HashMap<DeviceId, TrackedDevice> = HashMap::new();

    // TUI: one line per SN on top, a scrolling log at the bottom.
    let mut terminal = ratatui::init();
    let mut ui = Ui::new(registry.clone());

    // Terminal input on its own thread, forwarded into the select loop (event::read blocks).
    let (input_tx, input_rx) = crossbeam_channel::unbounded::<Event>();
    thread::spawn(move || {
        // event::read errors (terminal closed) end the loop; a closed receiver breaks it too.
        while let Ok(ev) = event::read() {
            if input_tx.send(ev).is_err() {
                break;
            }
        }
    });

    thread::spawn(move || {
        // Pure forwarder: no filtering or state, everything is handled by the main loop.
        // Create the watcher before enumerating, so devices plugged during enumeration aren't missed.
        let watcher = watch_devices().unwrap();

        // Emit already-connected devices at startup, not just future hotplug events.
        for dev in nusb::list_devices().wait().unwrap() {
            dev_tx.send(DeviceEvent::Connected(dev)).unwrap();
        }

        for event in block_on_stream(watcher) {
            match event {
                HotplugEvent::Connected(dev) => {
                    dev_tx.send(DeviceEvent::Connected(dev)).unwrap();
                }
                HotplugEvent::Disconnected(id) => {
                    dev_tx.send(DeviceEvent::Disconnected(id)).unwrap();
                }
            }
        }
    });

    terminal.draw(|f| ui.render(f)).unwrap();
    loop {
        crossbeam_channel::select! {
            recv(dev_rx) -> msg => {
                let Ok(msg) = msg else { break };
                match msg {
                    DeviceEvent::Connected(dev) => on_connected(
                        dev,
                        &mut serial_to_ids,
                        &mut id_to_device,
                        &mut active_workers,
                        &worker_tx,
                        &registry,
                        &mut ui,
                    ),
                    DeviceEvent::Disconnected(id) => on_disconnected(
                        id,
                        &mut serial_to_ids,
                        &mut id_to_device,
                        &active_workers,
                        &mut ui,
                    ),
                }
            }
            recv(worker_rx) -> msg => {
                let Ok(msg) = msg else { break };
                match msg {
                    WorkerEvent::Status(sn, phase, level, m) => ui.set_status(&sn, phase, level, m),
                    WorkerEvent::Died(sn, phase, level, reason) => {
                        active_workers.remove(&sn);
                        // Respawn only if a device with this SN is still present.
                        // Back off first, or repeated open failures become a die->respawn tight loop.
                        // The main loop can't sleep (would stall select), so a short-lived thread sends the sn back.
                        if serial_to_ids.contains_key(&Some(sn.clone())) {
                            // Worker gone but device present: show WaitingRestart with the death
                            // phase + reason. The restarted worker replaces it after the backoff.
                            ui.set_status(&sn, Phase::WaitingRestart, level, format!("[{phase}] {reason}"));
                            let respawn_tx = respawn_tx.clone();
                            thread::spawn(move || {
                                thread::sleep(RESPAWN_BACKOFF);
                                let _ = respawn_tx.send(sn);
                            });
                        } else {
                            // Device fully gone (already logged by on_disconnected): drop the line.
                            ui.remove(&sn);
                        }
                    }
                }
            }
            // Delayed respawn: rebuild the selector from the device currently tracked under this SN.
            recv(respawn_rx) -> msg => {
                let Ok(sn) = msg else { break };
                let selector = serial_to_ids
                    .get(&Some(sn.clone()))
                    .and_then(|ids| ids.first())
                    .and_then(|id| id_to_device.get(id))
                    .map(|tracked| DebugProbeSelector {
                        vendor_id: tracked.info.vendor_id(),
                        product_id: tracked.info.product_id(),
                        interface: None,
                        serial_number: tracked.serial.clone(),
                    });
                if let Some(selector) = selector {
                    // Log the restart; the worker reports its own status once running.
                    ui.log(format!("[SN: {sn}] 延遲重啟 worker"));
                    spawn_worker(sn, selector, &mut active_workers, &worker_tx, registry.clone());
                }
            }
            // Keyboard: UI owns the routing (screen state, firmware selection, path input); it
            // returns true to quit. Anything else just falls through to a redraw.
            recv(input_rx) -> ev => {
                let Ok(ev) = ev else { break };
                if let Event::Key(key) = ev
                    && key.kind == KeyEventKind::Press
                    && ui.handle_key(key)
                {
                    break;
                }
            }
        }
        terminal.draw(|f| ui.render(f)).unwrap();
    }

    ratatui::restore();
}
