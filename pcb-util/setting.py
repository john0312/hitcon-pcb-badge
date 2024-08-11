# -*- coding: utf-8 -*-
"""
Created on Sat Jul 13 08:16:40 2024

@author: Arthur (Tora0615)
"""
# Path
## For ReplaceELF

## For CLI tool
FW_ELF_PATH = "C:\\Users\\a8701\\Documents\\Development\\hitcon-pcb-badge\\pcb-util\\fwMOD.elf"
ST_PRO_PATH = (
    "C:\\Program Files (x86)\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin"
)
ST_PRO_EXE = "STM32_Programmer_CLI.exe"

## HTTP POST url
post_url = "http://servera.hitcon2024.online:7380/log_board"

# time relative (sec)
THREAD_SLEEP_INTERVAL = 2
CLI_QUIT_SCAN_INTERVAL = 0.1

MAX_ST_QTY = 10  # TODO : >10 device warning
CURSES_RESERVE_LINE = 1
