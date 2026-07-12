//! Per-board firmware placeholder injection, based on pcb-util/dev/ReplaceELF.py.
//!
//! PubKeyCert is intentionally NOT patched: this build has no provisioning server.

use memchr::memmem;
use rand::Rng;

use crate::ecc::{PRIV_KEY_LEN, gen_key};

// Placeholder patterns baked into the firmware.
const PER_BOARD_RANDOM: [u8; 16] = [
    0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d, //
    0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49,
];
const PRIV_KEY: [u8; PRIV_KEY_LEN] = [0x80, 0x02, 0xb6, 0x03, 0x60, 0xc6, 0x2f];

/// Returns how many non-overlapping occurrences of `pattern` were overwritten with `value`.
pub(crate) fn replace_all<const L: usize>(
    buf: &mut [u8],
    pattern: &[u8; L],
    value: &[u8; L],
) -> usize {
    let finder = memmem::Finder::new(pattern);
    let mut count = 0;
    let mut start = 0;
    while let Some(pos) = finder.find(&buf[start..]) {
        let at = start + pos;
        buf[at..at + L].copy_from_slice(value);
        start = at + L;
        count += 1;
    }
    count
}

/// Returned by [`inject`]; a replacement count of 0 means that placeholder was missing from the firmware.
pub(crate) struct Injected {
    pub(crate) image: Vec<u8>,
    /// (placeholder name, replacement count).
    pub(crate) replacements: Vec<(&'static str, usize)>,
}

/// Patch fresh per-board values into a copy of `firmware`.
pub(crate) fn inject(firmware: &[u8]) -> Injected {
    let mut image = firmware.to_vec();
    let mut random = [0u8; PER_BOARD_RANDOM.len()];
    rand::rng().fill_bytes(&mut random);
    let priv_key = gen_key(0); // team parity 0 for now
    let replacements = vec![
        (
            "PerBoardRandom",
            replace_all(&mut image, &PER_BOARD_RANDOM, &random),
        ),
        ("PrivKey", replace_all(&mut image, &PRIV_KEY, &priv_key)),
    ];
    Injected {
        image,
        replacements,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn count(buf: &[u8], pattern: &[u8]) -> usize {
        memmem::find_iter(buf, pattern).count()
    }

    #[test]
    fn replace_all_replaces_every_occurrence() {
        let pat = [0xAA, 0xBB, 0xCC];
        let val = [0x11, 0x22, 0x33];
        // Pattern at index 1 and 6; a trailing [AA,BB] partial that must be left alone.
        let mut buf = vec![
            0x00, 0xAA, 0xBB, 0xCC, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xAA, 0xBB,
        ];
        let orig_len = buf.len();
        let n = replace_all(&mut buf, &pat, &val);
        assert_eq!(n, 2);
        assert_eq!(buf.len(), orig_len, "length must not change");
        assert_eq!(count(&buf, &pat), 0, "all occurrences replaced");
        assert_eq!(count(&buf, &val), 2);
        assert_eq!(buf[0], 0x00);
        assert_eq!(buf[4], 0x99);
        assert_eq!(&buf[9..11], &[0xAA, 0xBB]);
    }
}
