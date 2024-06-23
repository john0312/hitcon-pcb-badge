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
- [ ] IR Tx: PWM Output from TIM3_CH3, Output PB0, DMA1 Ch2
  * Compare: 63/4 = 16
- [x] IR Rx: DMA Triggered through TIM2_CH3, Input PA0 read by DMA1 Ch1
  * Compare: 105
  * Mode: Normal Peripheral to Memory

|                   | Peripheral | Memory    |
| ----------------- | ---------- | --------- |
| Increment Address | X          | X         |
| Data Width        | Half Word  | Half Word |
- [x] Led Rows/Cols: DMA Triggered through TIM2_CH1, Output PB6-9(Rows), Output PB1-2/10-15(Cols), written by DMA1 Ch5
  * Compare: 315 
  * Mode: Circular Memory to Peripheral
  
|                   | Peripheral | Memory |
| ----------------- | ---------- | ------ |
| Increment Address | X          | V      |
| Data Width        | Word       | Word   |
- [x] Led Brightness control: PWM Output from TIM3_CH2
- [ ] Buttons: Reads Input PA4-10, PA15 through DMA1. Possible triggers.
  * Triggered through TIM3_CH4, DMA1 Ch3
  * Read together with IR Rx
  * Triggered through TIM4_CH2, DMA1 Ch4
- [x] Cross-board communication TX: USART2 TX, DMA1 Ch7
- [x] Cross-board communication RX: USART2 RX, DMA1 Ch6

- [ ] TIM4: Possibly operates the button at 500Hz.
- [x] TIM3: Operates at 38kHz for PWM out.
  * Prescaler: 5-1
  * Counter Period: 63-1
- [x] TIM2:
  * Prescaler: 3-1
  * Counter Period: 421-1
  * Operates at 1/4 speed of TIM3, synchronization needed for IR pulse alignment and brightness adjustment.
  * Also operates at 1/4 bit time for capturing IR.
  * TIM2 = 38/4 kHz = 9.5kHz.
  * Bit time = 4/9.5kHz = 421us (each bit is 16 pulse).
  * LED refreshes at 9.5kHz/16 = 593Hz.
  
