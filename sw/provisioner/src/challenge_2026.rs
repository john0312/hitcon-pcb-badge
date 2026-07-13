//! This flag serves one throwaway challenge and is not needed for normal firmware operation.
//! To remove the whole feature:
//!   1. delete this file
//!   2. delete `mod challenge_2026;` and the `challenge_2026::init(...)` call in main.rs
//!   3. delete the `// challenge_2026:` block in worker.rs `flash()` (revert `let mut injected`)
//!   4. (optional) drop `flag.txt` from .gitignore, and revert `replace_all` to private in inject.rs

use std::env::{self, VarError};
use std::fs;
use std::io::ErrorKind;
use std::sync::OnceLock;

use probe_rs::{MemoryInterface, Session};

use crate::inject::replace_all;
use crate::ui::Ui;

/// Env var holding the flag body directly (takes priority over the file).
const FLAG_ENV: &str = "PROVISIONER_FLAG";
/// Fallback file, read only when the env var is unset (relative to the working directory).
const FLAG_FILE: &str = "flag.txt";

/// Length of the flag body: the 12 chars inside FLAG{...} (understand[5..17]).
const FLAG_LEN: usize = 12;
type FlagBody = [u8; FLAG_LEN];

/// `desert_you`'s initial value in NeverGonna.cc ("@@so_do_i@@" + NUL).
const PLACEHOLDER: [u8; FLAG_LEN] = *b"@@so_do_i@@\0";
/// STM32 factory UID base; 12 bytes, same as `st-flash read ... 0x1FFFF7E8 12`.
const UID_BASE: u64 = 0x1FFF_F7E8;

/// Loaded once and cached. Err = a source was present but malformed.
static FLAG: OnceLock<Result<Option<FlagBody>, String>> = OnceLock::new();

/// What [`apply`] did to the image.
pub(crate) enum Applied {
    Injected(usize),    // placeholder replaced count times (count >= 1)
    PlaceholderMissing, // flag configured but this firmware has no desert_you placeholder
    NoFlag,             // no flag configured (env + file both absent)
}

/// Load the flag once and log the outcome. Optional: [`apply`] lazy-loads too, but calling this
/// at startup surfaces a misconfigured flag before any board is flashed.
pub(crate) fn init(ui: &mut Ui) {
    match flag() {
        Ok(Some(_)) => ui.log("已載入 flag，將逐板加密注入"),
        Ok(None) => ui.log(format!(
            "未設定 flag（環境變數 {FLAG_ENV} 或 {FLAG_FILE} 皆無），不注入 flag"
        )),
        Err(e) => ui.log(format!("flag 設定錯誤：{e}；不注入 flag")),
    }
}

/// Per-board: if a flag is configured, read this board's UID, compute
/// `desert_you = UID ^ flag_body`, and patch it into `image`.
pub(crate) fn apply(image: &mut [u8], session: &mut Session) -> Result<Applied, String> {
    let flag = match flag() {
        Ok(Some(flag)) => flag,
        Ok(None) => return Ok(Applied::NoFlag),
        Err(e) => return Err(e.clone()),
    };
    let uid = read_uid(session).map_err(|e| format!("讀取 UID 失敗：{e}"))?;
    let encrypted = xor(&uid, flag);
    match replace_all(image, &PLACEHOLDER, &encrypted) {
        0 => Ok(Applied::PlaceholderMissing),
        n => Ok(Applied::Injected(n)),
    }
}

/// Cached flag accessor; loads (env -> file) on first call.
fn flag() -> &'static Result<Option<FlagBody>, String> {
    FLAG.get_or_init(load_flag)
}

/// Read the flag body. Priority: env `PROVISIONER_FLAG`, then file `flag.txt`.
fn load_flag() -> Result<Option<FlagBody>, String> {
    match env::var(FLAG_ENV) {
        Ok(s) => return parse_body(s.as_bytes(), FLAG_ENV).map(Some),
        Err(VarError::NotUnicode(_)) => return Err(format!("{FLAG_ENV} 不是有效的 UTF-8")),
        Err(VarError::NotPresent) => {} // fall back to the file
    }
    match fs::read(FLAG_FILE) {
        Ok(bytes) => parse_body(&bytes, FLAG_FILE).map(Some),
        Err(e) if e.kind() == ErrorKind::NotFound => Ok(None),
        Err(e) => Err(format!("讀取 {FLAG_FILE} 失敗：{e}")),
    }
}

/// Strip one trailing line ending, then require exactly FLAG_LEN bytes.
fn parse_body(raw: &[u8], src: &str) -> Result<FlagBody, String> {
    let body = raw.strip_suffix(b"\n").unwrap_or(raw);
    let body = body.strip_suffix(b"\r").unwrap_or(body);
    if body.len() != FLAG_LEN {
        return Err(format!(
            "{src} 的 flag 長度應為 {FLAG_LEN} bytes，實際 {}",
            body.len()
        ));
    }
    let mut out = [0u8; FLAG_LEN];
    out.copy_from_slice(body);
    Ok(out)
}

/// Byte-wise XOR. Matches the firmware's per-word UID XOR (NeverGonna.cc LetYouDown) and
/// encrypt.py; endianness cancels because XOR is byte-parallel.
fn xor(a: &FlagBody, b: &FlagBody) -> FlagBody {
    std::array::from_fn(|i| a[i] ^ b[i])
}

/// Read the 12-byte factory UID from the attached target.
fn read_uid(session: &mut Session) -> Result<[u8; FLAG_LEN], probe_rs::Error> {
    let mut core = session.core(0)?;
    let mut uid = [0u8; FLAG_LEN];
    core.read_8(UID_BASE, &mut uid)?;
    Ok(uid)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xor_matches_reference_vector() {
        // Byte-for-byte with encrypt.py (uid used as the XOR key).
        let uid: FlagBody = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];
        let data: FlagBody = [
            0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        ];
        assert_eq!(
            xor(&uid, &data),
            [
                0xdf, 0xaf, 0xbd, 0xeb, 0x04, 0x25, 0x42, 0x6f, 0x80, 0xa1, 0xc6, 0xe3
            ]
        );
    }

    #[test]
    fn parse_accepts_exact_and_trailing_newline() {
        let want: FlagBody = *b"texttexttext";
        assert_eq!(parse_body(b"texttexttext", "t").unwrap(), want);
        assert_eq!(parse_body(b"texttexttext\n", "t").unwrap(), want);
        assert_eq!(parse_body(b"texttexttext\r\n", "t").unwrap(), want);
    }

    #[test]
    fn parse_rejects_wrong_length() {
        assert!(parse_body(b"too_short", "t").is_err());
        assert!(parse_body(b"this_is_too_long", "t").is_err());
    }
}
