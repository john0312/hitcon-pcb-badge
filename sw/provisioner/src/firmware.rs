//! Firmware registry: which firmware each hardware year can flash, and how to load it.
//!
//! Every year keeps its own list of choices (an embedded default plus any added at runtime) and
//! remembers which one is selected; the active year decides what gets flashed. Shared with the
//! workers as `Arc<Mutex<FwRegistry>>`; they read the active choice at flash time.

use std::borrow::Cow;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

// Embedded firmware, baked into the binary (no runtime file dependency).
const FW_2024: &[u8] = include_bytes!("../../../fw/V1.1/fw.elf");
const FW_2025: &[u8] = include_bytes!("../../../fw/V2.2 RELEASE/fw.elf");

/// Which year is active on startup (index into `FwRegistry::years`). Flip to 0 for 2024.
const DEFAULT_ACTIVE: usize = 1; // 2025

/// The hardware version being provisioned.
#[derive(Clone, Copy)]
pub(crate) enum HwYear {
    Y2024,
    Y2025,
}

impl HwYear {
    /// Short tab label shown in the UI.
    pub(crate) fn label(self) -> &'static str {
        match self {
            HwYear::Y2024 => "2024",
            HwYear::Y2025 => "2025",
        }
    }
}

/// Where a firmware's bytes come from.
#[derive(Clone)]
pub(crate) enum FirmwareSource {
    Embedded(&'static [u8]), // baked in via include_bytes!
    File(PathBuf),           // added at runtime by path
}

impl FirmwareSource {
    /// Embedded returns the baked-in slice; File re-reads from disk each call, so a rebuilt
    /// firmware is picked up without re-adding it.
    pub(crate) fn load(&self) -> io::Result<Cow<'static, [u8]>> {
        match self {
            FirmwareSource::Embedded(bytes) => Ok(Cow::Borrowed(bytes)),
            FirmwareSource::File(path) => Ok(Cow::Owned(fs::read(path)?)),
        }
    }
}

pub(crate) struct FwChoice {
    pub(crate) label: String,
    pub(crate) source: FirmwareSource,
}

/// A year's firmware choices; `choices[0]` is the embedded default, `selected` indexes into them.
pub(crate) struct YearFw {
    pub(crate) year: HwYear,
    pub(crate) choices: Vec<FwChoice>,
    pub(crate) selected: usize,
}

pub(crate) struct FwRegistry {
    pub(crate) years: [YearFw; 2], // [0] = 2024, [1] = 2025
    pub(crate) active: usize,
}

impl Default for FwRegistry {
    fn default() -> Self {
        FwRegistry {
            years: [
                YearFw {
                    year: HwYear::Y2024,
                    choices: vec![FwChoice {
                        label: "內建-hw1.1".to_string(),
                        source: FirmwareSource::Embedded(FW_2024),
                    }],
                    selected: 0,
                },
                YearFw {
                    year: HwYear::Y2025,
                    choices: vec![FwChoice {
                        label: "內建-hw2.2".to_string(),
                        source: FirmwareSource::Embedded(FW_2025),
                    }],
                    selected: 0,
                },
            ],
            active: DEFAULT_ACTIVE,
        }
    }
}

impl FwRegistry {
    /// The year currently selected for flashing.
    pub(crate) fn active_year(&self) -> &YearFw {
        &self.years[self.active]
    }

    /// The firmware choice that would be flashed right now.
    pub(crate) fn active_choice(&self) -> &FwChoice {
        let year = self.active_year();
        &year.choices[year.selected]
    }

    /// Move to the previous / next year (clamped; ← lands on 2024, → on 2025).
    pub(crate) fn prev_year(&mut self) {
        self.active = self.active.saturating_sub(1);
    }
    pub(crate) fn next_year(&mut self) {
        self.active = (self.active + 1).min(self.years.len() - 1);
    }

    /// Move the selection cursor within the active year (clamped).
    pub(crate) fn select_prev(&mut self) {
        let year = &mut self.years[self.active];
        year.selected = year.selected.saturating_sub(1);
    }
    pub(crate) fn select_next(&mut self) {
        let year = &mut self.years[self.active];
        if year.selected + 1 < year.choices.len() {
            year.selected += 1;
        }
    }

    /// Validate `path` is readable, then add it to the active year and select it. The bytes are
    /// re-read at flash time, not cached here; the read here is only to reject bad paths early.
    pub(crate) fn add_to_active(&mut self, path: &str) -> io::Result<()> {
        let path = expand_tilde(path);
        fs::read(&path)?; // validate readable now (also rejects directories); discard the bytes
        let label = path
            .file_name()
            .unwrap_or(path.as_os_str())
            .to_string_lossy()
            .into_owned();
        let year = &mut self.years[self.active];
        year.choices.push(FwChoice {
            label,
            source: FirmwareSource::File(path),
        });
        year.selected = year.choices.len() - 1;
        Ok(())
    }
}

/// Expand a leading `~/` to `$HOME`; other paths pass through unchanged.
fn expand_tilde(path: &str) -> PathBuf {
    if let Some(rest) = path.strip_prefix("~/")
        && let Ok(home) = std::env::var("HOME")
    {
        return Path::new(&home).join(rest);
    }
    PathBuf::from(path)
}
