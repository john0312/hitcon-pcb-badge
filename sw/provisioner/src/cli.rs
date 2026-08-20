use clap::Parser;
use std::sync::LazyLock;

#[derive(Parser)]
#[command(about = "HITCON badge provisioner")]
pub(crate) struct Args {
    /// 無序號 ST-Link 當成唯一的一支處理
    // Windows can't read ST-Link V2's binary serial, so those devices would otherwise never get a
    // worker (see note.md).
    #[arg(long)]
    pub(crate) single_probe: bool,
}

/// Forced early in main(), before ratatui takes the screen: clap prints --help and argument errors
/// to stdout and exits, which would be invisible inside the alternate screen.
pub(crate) static ARGS: LazyLock<Args> = LazyLock::new(Args::parse);
