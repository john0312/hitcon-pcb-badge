// Verify full attach (the Session path used for flashing): open -> attach -> Session -> drop -> reopen.
// Reopening every time proves the Session drop releases the ST-Link.
use probe_rs::Permissions;
use probe_rs::probe::list::Lister;
use std::time::Instant;

const TARGET: &str = "STM32F103CBT6";

fn main() {
    let lister = Lister::new();
    let Some(info) = lister.list_all().into_iter().next() else {
        println!("找不到 probe，插上 ST-Link 再跑");
        return;
    };
    println!("probe: {}\n", info.identifier);

    for i in 0..5 {
        let probe = match info.open() {
            Ok(p) => p,
            Err(e) => {
                println!("[{i}] open FAIL: {e}");
                continue;
            }
        };
        let t = Instant::now();
        match probe.attach(TARGET, Permissions::default()) {
            Ok(session) => {
                println!(
                    "[{i}] attach OK  ({:?})  target={}，drop 釋放",
                    t.elapsed(),
                    session.target().name
                );
                drop(session); // mimic end of flash
            }
            Err(e) => println!("[{i}] attach ERR ({:?})  {e:?}", t.elapsed()),
        }
    }
    println!("\ndone");
}
