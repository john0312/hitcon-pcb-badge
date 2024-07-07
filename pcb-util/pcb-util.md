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