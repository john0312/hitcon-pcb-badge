# HITCON PCB Badge

## Project Setup

After cloning the repository, execute the following commands to prevent the environment from being checked by Git.

```shell
git config --local filter.eclipse_env_hash.clean "sed 's/env-hash=\"[^\"]*\"/env-hash=\"\"/g'"
git config --local filter.eclipse_env_hash.smudge cat
```

ref: [Eclipse Community Forums: C / C++ IDE (CDT) &raquo; Keeping language.settings.xml under source control?](https://www.eclipse.org/forums/index.php/t/1074031/)

## Firmware Development

execute `fw/merge.py` to merge STM32CubeIDE generated `main.c` with user code in `main.cc` into `main.cc`.

## Timers and DMA Channels
- IR Tx: Timer 3, Channel 3, Output PB0
- IR Rx: Timer 2, Combined Channel 1, Input PA0
- Led Rows: Timer 2, Channel 1, Output PB6-9
- Led Cols: Timer 2, Channel 1, Output PB1-2, 10-15
- Buttons: Timer 3, Channel 1, Input PA4-10, PA15 (To be verified)
- Cross-board communication: Timer 4 (To be verified)
