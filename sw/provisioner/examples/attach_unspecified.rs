// Verify attach_to_unspecified (borrow &mut, no Session) as a board-present check:
// open once, poll via borrow-reuse without reopening. Ok = present, Err = no response.
// Unplug/replug the target while it runs to watch OK <-> ERR.
use probe_rs::probe::list::Lister;
use std::time::Instant;

fn main() {
    let lister = Lister::new();
    let Some(info) = lister.list_all().into_iter().next() else {
        println!("找不到 probe，插上 ST-Link 再跑");
        return;
    };
    println!("probe: {}\n", info.identifier);

    let mut probe = match info.open() {
        Ok(p) => p,
        Err(e) => {
            println!("open FAIL: {e}");
            return;
        }
    };
    for i in 0..10 {
        let t = Instant::now();
        let r = probe.attach_to_unspecified();
        let dt = t.elapsed();
        if r.is_ok() {
            let _ = probe.detach();
        }
        match r {
            Ok(()) => println!("[{i}] OK  ({dt:?})  板子在"),
            Err(e) => println!("[{i}] ERR ({dt:?})  板子不在?  {e:?}"),
        }
    }
    println!("\ndone — probe 全程只 open 一次");
}
