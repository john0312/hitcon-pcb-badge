# Description
pcb-util provides an utility tool for flashing FW to PCBs more effiecient in production.

# Build
## UI
[Mesop of Python](https://google.github.io/mesop/)

## Upload Process
Python 3.12
### Interface
LoadModifier(Modifier, MarkToModify)
    Configure the modfier and position to modify in .hex file
    - Modifier: part of .hex to be replaced with 
    - MarkToModify: position of .hex to be replaced

ModifyFwHex(FwHexPath, FwHexModPath, Modifier, MarkToModify)
    Modify the .hex and output a modified .hex
    - FwHexPath: .hex path of FW build
    - FwHexModPath: modified .hex of FW build
    - Modifier: part of .hex to be replaced with 
    - MarkToModify: position of .hex to be replaced

FlashHex(STLinkCLIPath, FwHexModPath, STLinkSN)
    Flash the modified .hex thorugh the target STLink

FlashStatus()
    Return the status flashing
    - Standby: 0
    - Downloading:1
    - Reading: 2
    - Verifying: 3

### Hex operation
Some part of the FW .HEX file(to be marked by John) to be replaced before flashing

## Multi-drop
TBD

-----------------------

## upload process of a sigle set

> the st-link is connected

1. detect the board is exist or not
2. if exist, auto trigger upload, verify, start


# Some notes

ref
https://blog.csdn.net/yxy244/article/details/108453398

## need an exe selector
* check exe file correct or not

> normal path : 
> C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe

## list all stlink devices
```
PS C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin> ./STM32_Programmer_CLI.exe -l
      -------------------------------------------------------------------
                       STM32CubeProgrammer v2.17.0
      -------------------------------------------------------------------

=====  DFU Interface   =====

No STM32 device in DFU mode connected

===== J-Link Interface =====
Error: No J-link probe detected.
===== STLink Interface =====
ST-LINK error (DEV_CONNECT_ERR)

-------- Connected ST-LINK Probes List --------

ST-Link Probe 0 :
   ST-LINK SN  : 5&1E237D3C&0&3
   ST-LINK FW  :
   Access Port Number  : 0
   Board Name  :

ST-Link Probe 1 :
   ST-LINK SN  : 56FF6F066682495259282187
   ST-LINK FW  : V2J45S7
   Access Port Number  : 1
   Board Name  :
-----------------------------------------------

=====  UART Interface  =====

Total number of serial ports available: 0

```

## get device info

### success

```
PS C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin> ./STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187
      -------------------------------------------------------------------
                       STM32CubeProgrammer v2.17.0
      -------------------------------------------------------------------

ST-LINK SN  : 56FF6F066682495259282187
ST-LINK FW  : V2J45S7
Board       : --
Voltage     : 3.23V
SWD freq    : 4000 KHz
Connect mode: Normal
Reset mode  : Software reset
Device ID   : 0x410
Revision ID : Rev X
Device name : STM32F101/F102/F103 Medium-density
Flash size  : 64 KBytes
Device type : MCU
Device CPU  : Cortex-M3
BL Version  : --
```

### failed
```
./STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187
      -------------------------------------------------------------------
                       STM32CubeProgrammer v2.17.0
      -------------------------------------------------------------------

ST-LINK SN  : 56FF6F066682495259282187
ST-LINK FW  : V2J45S7
Board       : --
Voltage     : 3.23V
Error: No STM32 target found! If your product embeds Debug Authentication, please perform a discovery using Debug Authentication
```
