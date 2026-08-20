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

/// Worker/UI key standing in for the unreadable serial under --single-probe.
/// ASCII so the SN column stays aligned.
const SINGLE_PROBE_KEY: &str = "SINGLE-PROBE";

/// Watcher thread -> main: raw hotplug events only; all state lives in the main loop.
pub(crate) enum DeviceEvent {
    Connected(DeviceInfo),
    Disconnected(DeviceId),
}

/// A tracked device: its serial (formatted for open() matching) and the raw DeviceInfo.
pub(crate) struct TrackedDevice {
    pub(crate) serial: Option<String>,
    /// Worker/UI key. Same as `serial`, except under --single-probe where a serial-less device
    /// gets SINGLE_PROBE_KEY. Kept apart from `serial` so the probe-rs selector still sees None
    /// and matches on VID/PID alone.
    pub(crate) key: Option<String>,
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
    let key = sn.clone().or_else(|| {
        crate::cli::ARGS
            .single_probe
            .then(|| SINGLE_PROBE_KEY.to_string())
    });
    // A device with a key gets a table line; a no-serial one is log-only (can't key a worker).
    match &key {
        Some(s) => ui.set_status(s, Phase::ProbeConnected, Level::Info, "新裝置已連接"),
        None => ui.log("偵測到無序號 ST-Link 裝置"),
    }
    let same_key = serial_to_ids.entry(key.clone()).or_default();
    same_key.push(id);
    // --single-probe assumes exactly one probe. A second serial-less one lands on the same key and
    // the selector matches on VID/PID alone, so probe-rs would pick between them arbitrarily.
    if key.as_deref() == Some(SINGLE_PROBE_KEY) && same_key.len() > 1 {
        ui.log(format!(
            "警告：--single-probe 下有 {} 支無序號裝置，無法確定燒錄目標，請只留一支",
            same_key.len()
        ));
    }

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
            key: key.clone(),
            info: dev,
        },
    );

    // No key -> can't key a worker; record only.
    let Some(key) = key else {
        return;
    };
    // Give USB/probe-rs a moment to enumerate before opening.
    thread::sleep(Duration::from_millis(200));
    spawn_worker(key, selector, active_workers, worker_tx, registry.clone());
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
    let sn = tracked.key;
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
