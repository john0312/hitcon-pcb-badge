# pcb-util

pcb-util provides an utility tool for flashing FW to PCBs more effiecient in production.

## Dev env
- Python ver
  * python 3.12
### Packages ver
- UI
  - [Streamlit](https://streamlit.io/)
      - 1.37.0
- Upload Process
  - STM32_Programmer_CLI
    -  2.17.0
-  TODO: EXE generation
   - pyinstaller
     - 6.9.0

## Application
The pcb-util consist of several utility scripts describing as follows
- **ReplaceELF.py (Done)**
  1. Duplicate the compiled ELF as fwMOD.elf to be modified
  2. Find & Replace the two uint8 arrays in fwMOD.elf with specific array for individual pcb-badge
  3. Printe the array around in organized HEX to verify ELF modification
- **fw_flash.py (Debugging)**
  - Class definition for automating the interaction with STM32_Programmer_CLI & ST-Link
- **setting.py**
  - Some configurations for fw_flash
- **cli_example.py (Debugging)**
  - An example of a multi-thread CLI app that handles each ST-Link
  - Some knwown bug can be fix with some notes [here](#faq-of-cli_example)
- **ui.py**
  - A streamlit app which spawns a web server
  - Check out [here](#try-out-ui) if your want to try it out
- **ui_example.py (Under contruction)**
  - A streamlit app of a multi-thread CLI app that handles each ST-Link
  - TODO: display connected ST-link
  - TODO: auto-detect ST-Link

## Try out CLI
to try it out:
1. Run:
    ```
    python cli_example.py
    ```
2. A CLI app would be running with following keystroke as input
- press r', refresh the ST-Link list
- 'q', quit the app

## Try out UI
[Streamlit](https://streamlit.io/)
to try it out:
1. Set up your Python development environment.
2. Run:
    ```
    pip install streamlit
    ```
3. Validate the installation by running official Hello app:
    ```
    streamlit hello
    ```
4. Point to directory of \hitcon-pcb-badge\pcb-util and run the UI app for HITCON
    ```
    streamlit ui.py
    ```


## FAQ of cli_example
* curses 出錯 : 
![alt text](image.png)
  * 因留給他的空間不夠，顯示空間拉大就好
* spyder 終端機跳掉
  * 用 vscode 執行
  
## Some notes

### ref links

* [STM32_Programmer_CLI.exe基本命令介绍](https://blog.csdn.net/yxy244/article/details/108453398)

### issue list
