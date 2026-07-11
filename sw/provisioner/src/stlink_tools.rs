use std::collections::HashMap;
use std::fmt::Write;
use std::sync::LazyLock;

// copied from probe-rs/src/probe/stlink/tools.rs
pub fn is_stlink_device(device: &nusb::DeviceInfo) -> bool {
    // Check the VID/PID.
    (device.vendor_id() == USB_VID) && (USB_PID_EP_MAP.contains_key(&device.product_id()))
}

// serial number function must be the same
pub fn read_serial_number(device: &nusb::DeviceInfo) -> Option<String> {
    device.serial_number().map(|s| {
        if s.len() < 24 {
            // Some STLink (especially V2) have their serial number stored as a 12 bytes binary string
            // containing non printable characters, so convert to a hex string to make them printable.
            to_hex(s)
        } else {
            // Other STlink (especially V2-1) have their serial number already stored as a 24 characters
            // hex string so they don't need to be converted
            s.to_string()
        }
    })
}
fn to_hex(s: &str) -> String {
    s.as_bytes().iter().fold(String::new(), |mut s, b| {
        let _ = write!(s, "{b:02X}"); // Writing a String never fails
        s
    })
}

// copied from probe-rs/src/probe/stlink/usb_interface.rs
/// The USB VendorID.
pub const USB_VID: u16 = 0x0483;

/// Map of USB PID to firmware version name and device endpoints.
pub static USB_PID_EP_MAP: LazyLock<HashMap<u16, StLinkInfo>> = LazyLock::new(|| {
    let mut m = HashMap::new();
    m.insert(0x3748, StLinkInfo::new("V2", 0x02, 0x81, 0x83));
    m.insert(0x374b, StLinkInfo::new("V2-1", 0x01, 0x81, 0x82));
    m.insert(0x374a, StLinkInfo::new("V2-1", 0x01, 0x81, 0x82)); // Audio
    m.insert(0x3742, StLinkInfo::new("V2-1", 0x01, 0x81, 0x82)); // No MSD
    m.insert(0x3752, StLinkInfo::new("V2-1", 0x01, 0x81, 0x82)); // Unproven
    m.insert(0x374e, StLinkInfo::new("V3", 0x01, 0x81, 0x82));
    m.insert(0x374f, StLinkInfo::new("V3", 0x01, 0x81, 0x82)); // Bridge
    m.insert(0x3753, StLinkInfo::new("V3", 0x01, 0x81, 0x82)); // 2VCP
    m.insert(0x3754, StLinkInfo::new("V3", 0x01, 0x81, 0x82)); // Without mass storage
    m.insert(0x3757, StLinkInfo::new("V3PWR", 0x01, 0x81, 0x82)); // Bridge and power, no MSD
    m
});

/// A helper struct to match STLink device info.
#[allow(dead_code)]
#[derive(Clone, Debug, Default)]
pub struct StLinkInfo {
    pub version_name: &'static str,
    ep_out: u8,
    ep_in: u8,
    ep_swo: u8,
}

impl StLinkInfo {
    pub const fn new(version_name: &'static str, ep_out: u8, ep_in: u8, ep_swo: u8) -> Self {
        Self {
            version_name,
            ep_out,
            ep_in,
            ep_swo,
        }
    }
}
