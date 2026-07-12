use nusb::{DeviceId, DeviceInfo};
use probe_rs::probe::DebugProbeSelector;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use crate::firmware::FwRegistry;
use crate::stlink_tools::{is_stlink_device, read_serial_number};
use crate::ui::Ui;
use crate::worker::{Level, Phase, WorkerCmd, WorkerEvent, spawn_worker};

/// Watcher thread -> main: raw hotplug events only; all state lives in the main loop.
pub(crate) enum DeviceEvent {
    Connected(DeviceInfo),
    Disconnected(DeviceId),
}

/// A tracked device: its serial (formatted for open() matching) and the raw DeviceInfo.
pub(crate) struct TrackedDevice {
    pub(crate) serial: Option<String>,
    pub(crate) info: DeviceInfo,
}

/// Device connected: filter ST-Links, record state, spawn a worker.
/// Shared by startup enumeration and hotplug.
pub(crate) fn on_connected(
    dev: DeviceInfo,
    serial_to_ids: &mut HashMap<Option<String>, Vec<DeviceId>>,
    id_to_device: &mut HashMap<DeviceId, TrackedDevice>,
    active_workers: &mut HashMap<String, crossbeam_channel::Sender<WorkerCmd>>,
    worker_tx: &crossbeam_channel::Sender<WorkerEvent>,
    registry: &Arc<Mutex<FwRegistry>>,
    ui: &mut Ui,
) {
    if !is_stlink_device(&dev) {
        return;
    }
    let id = dev.id();
    // Enumeration and hotplug can report the same device twice.
    if id_to_device.contains_key(&id) {
        return;
    }
    // Same serial algorithm as probe-rs, so it matches list_probes_filtered.
    let sn = read_serial_number(&dev);
    // A device with a serial gets a table line; a no-serial one is log-only (can't key a worker).
    match &sn {
        Some(s) => ui.set_status(s, Phase::ProbeConnected, Level::Info, "新裝置已連接"),
        None => ui.log("偵測到無序號 ST-Link 裝置"),
    }
    serial_to_ids.entry(sn.clone()).or_default().push(id);

    // Build the selector while we still own dev (before it moves into the map).
    let selector = DebugProbeSelector {
        vendor_id: dev.vendor_id(),
        product_id: dev.product_id(),
        interface: None,
        serial_number: sn.clone(),
    };
    id_to_device.insert(
        id,
        TrackedDevice {
            serial: sn.clone(),
            info: dev,
        },
    );

    // No serial -> can't key a worker; record only.
    let Some(sn) = sn else {
        return;
    };
    // Give USB/probe-rs a moment to enumerate before opening.
    thread::sleep(Duration::from_millis(200));
    spawn_worker(sn, selector, active_workers, worker_tx, registry.clone());
}

/// Device disconnected: update state and signal the worker based on how many of the same SN remain.
/// Only sends commands; removal from active_workers is left to the worker's Died event.
pub(crate) fn on_disconnected(
    id: DeviceId,
    serial_to_ids: &mut HashMap<Option<String>, Vec<DeviceId>>,
    id_to_device: &mut HashMap<DeviceId, TrackedDevice>,
    active_workers: &HashMap<String, crossbeam_channel::Sender<WorkerCmd>>,
    ui: &mut Ui,
) {
    // Not tracked -> not our ST-Link.
    let Some(tracked) = id_to_device.remove(&id) else {
        return;
    };
    let sn = tracked.serial;
    // Drop this id and count how many remain under the same sn.
    let remaining = match serial_to_ids.get_mut(&sn) {
        Some(ids) => {
            ids.retain(|&i| i != id);
            let n = ids.len();
            if n == 0 {
                serial_to_ids.remove(&sn);
            }
            n
        }
        None => 0,
    };
    let Some(sn) = sn else {
        return;
    };
    // No worker for this SN: nothing will emit Died, so if it's fully gone remove the row here.
    let Some(cmd_tx) = active_workers.get(&sn) else {
        if remaining == 0 {
            ui.log(format!("[SN: {sn}] 全部移除（無運作中的 worker）"));
            ui.remove(&sn);
        }
        return;
    };
    if remaining == 0 {
        // A command, not a phase -> log only. The line is removed once the worker acts on Kill
        // and dies (see the Died handler in main).
        ui.log(format!("[SN: {sn}] 全部移除，通知 worker 結束"));
        let _ = cmd_tx.send(WorkerCmd::Kill);
    } else {
        // Same SN still has a device -> have the worker re-check itself (a command, not a phase).
        ui.log(format!(
            "[SN: {sn}] 移除一支但仍有裝置，通知 worker 重新檢查"
        ));
        let _ = cmd_tx.send(WorkerCmd::WakeUp);
    }
}
