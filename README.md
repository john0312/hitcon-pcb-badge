# HITCON PCB Badge

## Firmware Development

execute `fw/merge.py` to merge STM32CubeIDE generated `main.c` with user code in `main.cc` into `main.cc`.

### Choose your hardware

In STM32CubeIDE, go to **Project > Build Configurations > Set Active > V1.1 / V2.0 / V2.1** to switch between hardware versions.  
Also go to **Run > Run Configurations > C/C++ Application** and choose the correct Build Configuration
There are different firmware versions for each hardware revision.  
To identify your hardware version, check the back of your badge.
- V1.1 (2024 CMT Attendee)
- V2.0 (2025 first prototype)
- V2.1 (2025 CMT Attendee)



## Timers and DMA Channels

- [x] IR Tx: PWM Output from TIM3_CH3, Output PB0, DMA1 Ch2
  - Compare: 63/4 = 16
  - Mode: Circular Normal Peripheral to Memory

|                   | Peripheral | Memory    |
| ----------------- | ---------- | --------- |
| Increment Address | X          | V         |
| Data Width        | Half Word  | Half Word |

- [x] IR Rx: DMA Triggered through TIM2_CH3, Input PA0 read by DMA1 Ch1
  - Compare: 105
  - Mode: Circular Peripheral to Memory

|                   | Peripheral | Memory    |
| ----------------- | ---------- | --------- |
| Increment Address | X          | V         |
| Data Width        | Half Word  | Half Word |

- [x] Led Rows/Cols: DMA Triggered through TIM1_UP, Output PB6-9(Rows), Output PB1-2/10-15(Cols), written by DMA1 Ch5
  - Mode: Circular Memory to Peripheral

|                   | Peripheral | Memory |
| ----------------- | ---------- | ------ |
| Increment Address | X          | V      |
| Data Width        | Word       | Word   |

- [x] Led Brightness control: PWM Output from TIM3_CH2
- [x] Buttons: Reads Input PA4-10, PA15 through TIM4_CH2, DMA1 Ch4
  - Compare: 5
  - Mode: Circular Peripheral to Memory

|                   | Peripheral | Memory    |
| ----------------- | ---------- | --------- |
| Increment Address | X          | V         |
| Data Width        | Half Word  | Half Word |

- [x] Cross-board communication TX: USART2 TX, DMA1 Ch7
- [x] Cross-board communication RX: USART2 RX, DMA1 Ch6

- [x] TIM4: Operates at 100Hz for Button
  - Prescaler: 12000-1
  - Counter Period: 10-1
- [x] TIM3: Operates at 38kHz for PWM out.
  - Prescaler: 5-1
  - Counter Period: 63-1
  - Master Mode Enable
- [x] TIM2:
  - Prescaler: 0
  - Counter Period: 4-1
  - Slave Mode: External Clock
  - Trigger Source: ITR2 (TIM3 Update Event)
  - Operates at 1/4 speed of TIM3, synchronization needed for IR pulse alignment and brightness adjustment.
  - Also operates at 1/4 bit time for capturing IR.
  - TIM2 = 38/4 kHz = 9.5kHz.
  - Bit time = 4/9.5kHz = 421us (each bit is 16 pulse).
- [x] TIM1:
  - Prescaler: 5-1
  - Counter Period: 1500-1
  - LED refreshes at 3200/16 = 200Hz.

## Format

To check format, please install `clang-format` first.

```sh
# under hitcon-pcb-badge
./lint.sh # check format
./lint.sh -i # auto format
```
