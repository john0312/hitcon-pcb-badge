use futures::executor::block_on_stream;
use nusb::watch_devices;
use nusb::{DeviceId, MaybeFuture, hotplug::HotplugEvent};
use probe_rs::probe::DebugProbeSelector;
use std::collections::HashMap;
use std::thread;
use std::time::Duration;

mod device;
mod stlink_tools;
mod worker;

use device::{DeviceEvent, TrackedDevice, on_connected, on_disconnected};
use worker::{WorkerCmd, WorkerEvent, spawn_worker};

/// How long to wait before respawning a worker whose device is still present (avoids a die->respawn tight loop).
const RESPAWN_BACKOFF: Duration = Duration::from_millis(500);

fn main() {
    let (dev_tx, dev_rx) = crossbeam_channel::unbounded::<DeviceEvent>();
    let (worker_tx, worker_rx) = crossbeam_channel::unbounded::<WorkerEvent>();
    // Backoff respawn requests: after Died with the device still present, send the sn back later.
    let (respawn_tx, respawn_rx) = crossbeam_channel::unbounded::<String>();

    let mut active_workers: HashMap<String, crossbeam_channel::Sender<WorkerCmd>> = HashMap::new();

    // Device state, centralized in the main loop so Died can look it up.
    // serial -> its dev ids (Vec only in case serials collide); None = no-serial devices.
    let mut serial_to_ids: HashMap<Option<String>, Vec<DeviceId>> = HashMap::new();
    // id -> device: Disconnected only gives an id, so this reverse-maps to the serial (and keeps DeviceInfo).
    let mut id_to_device: HashMap<DeviceId, TrackedDevice> = HashMap::new();

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
                    ),
                    DeviceEvent::Disconnected(id) => on_disconnected(
                        id,
                        &mut serial_to_ids,
                        &mut id_to_device,
                        &active_workers,
                    ),
                }
            }
            recv(worker_rx) -> msg => {
                let Ok(msg) = msg else { break };
                match msg {
                    WorkerEvent::Status(sn, m) => println!("[SN: {}] {}", sn, m),
                    WorkerEvent::Died(sn) => {
                        println!("❌ [SN: {}] Worker 結束", sn);
                        active_workers.remove(&sn);
                        // Respawn only if a device with this SN is still present.
                        // Back off first, or repeated open failures become a die->respawn tight loop.
                        // The main loop can't sleep (would stall select), so a short-lived thread sends the sn back.
                        if serial_to_ids.contains_key(&Some(sn.clone())) {
                            let respawn_tx = respawn_tx.clone();
                            thread::spawn(move || {
                                thread::sleep(RESPAWN_BACKOFF);
                                let _ = respawn_tx.send(sn);
                            });
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
                    println!("[SN: {}] 重啟延遲 worker", sn);
                    spawn_worker(sn, selector, &mut active_workers, &worker_tx);
                }
            }
        }
    }
}
