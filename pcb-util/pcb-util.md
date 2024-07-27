# pcb-util

## 0x00. Description

pcb-util provides an utility tool for flashing FW to PCBs more effiecient in production.

## 0x01. Dev env

### A. Python ver

* python 3.12

### B. Packages ver

#### a. UI

* [Mesop of Python](https://google.github.io/mesop/)

#### b. Upload Process

* 

#### c. exe generation

* pyinstaller

### C. Other requirment

* STM32_Programmer_CLI.exe



## 0x02. Process flow

### Pure cli

* You cann see **stm_cli_uploader.drawio**

![alt text](stm_cli_uploader.png)


## 0x03. Interface

### ????
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


### fw_flash.py
TBD

### Hex operation
Some part of the FW .HEX file(to be marked by John) to be replaced before flashing

## Multi-drop
TBD


# 常見問題

* curses 出錯 : 
![alt text](image.png)
  * 因留給他的空間不夠，顯示空間拉大就好
* spyder 終端機跳掉
  * 用 vscode 執行
----------------------
-



## Some notes

### ref links

* [STM32_Programmer_CLI.exe基本命令介绍](https://blog.csdn.net/yxy244/article/details/108453398)

### issue list




====

# Build
## UI
[Streamlit](https://streamlit.io/)
to try it out:
1. Set up your Python development environment.
2. Run:
    ```
    streamlit hello
    ```
3. Validate the installation by running official Hello app:
    ```
    streamlit hello
    ```
4. Point to directory of \hitcon-pcb-badge\pcb-util and run the UI app for HITCON
    ```
    streamlit ui.py
    ```