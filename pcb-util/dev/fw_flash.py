# -*- coding: utf-8 -*-
"""
Created on Sun Jul  7 19:07:46 2024

@author: Arthur (Tora0615)
"""

import re
import subprocess
import threading
import time
from enum import Enum, auto
import ReplaceELF
import configparser
import os
from typing import Tuple, ClassVar

# Global
flag_HTTPServerConnnectionError = False
flag_FwElfNotFound = False
PerBoardSecret = []
PrivKey: bytes = b''

# Read the .ini file
config = configparser.ConfigParser()
config.read('config.ini')

# Extract variables from the .ini file
FW_ELF_PATH = config.get('Paths', 'FW_ELF_PATH')
ST_PRO_PATH, ST_PRO_EXE = os.path.split(config.get('Paths', 'ST_PPROGRAMMER_PATH'))
post_url = config.get('HTTP', 'POST_URL')
THREAD_SLEEP_INTERVAL = config.getfloat('Settings', 'THREAD_SLEEP_INTERVAL')
CLI_QUIT_SCAN_INTERVAL = config.getfloat('Settings', 'CLI_QUIT_SCAN_INTERVAL')
MAX_ST_QTY = config.getint('Settings', 'MAX_ST_QTY')
CURSES_RESERVE_LINE = config.getint('Settings', 'CURSES_RESERVE_LINE')
EN_PCB_LOG = config.getint('HTTP', 'EN_PCB_LOG')


# status enum class
class ST_STATUS(Enum):
    NO_DEVICE = 0
    UPLOADING = auto()
    VERIFYING = auto()
    TRIGGER_EXEC = auto()
    FINISHED = auto()

#--- interface to use STM32CubeProgrammer by command---
def run_command(cmd) -> Tuple[str, str]:
    process = subprocess.Popen(
        cmd, shell=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, errors='ignore')
    stdout, stderr = process.communicate()
    return stdout, stderr

# data class
class ST_CONFIG():
    def __init__(self, ELF_PATH, ST_PRO_PATH, ST_PRO_EXE, PORT='SWD'):
        self.STLINK_PORT = PORT
        self.FW_ELF_PATH = ELF_PATH
        self.ST_PRO_PATH = ST_PRO_PATH
        self.ST_PRO_EXE = ST_PRO_EXE

    #--- commands to STM32CubeProgrammer, sn unrelated ---
    def gen_bin_path_command(self) -> str:
        cmd = f'"{self.ST_PRO_PATH}\\{self.ST_PRO_EXE}"'
        return cmd

    def gen_stlink_list_command(self) -> str:
        cmd = self.gen_bin_path_command() + " -l"
        return cmd

    #--- ---
    def list_stlink(self) -> list:
        out, err = run_command(self.gen_stlink_list_command())

        # stderr check
        if not err:
            # regex
            pattern = r"ST-LINK SN\s+:\s+(\S+)"
            matches = re.findall(pattern, out)

            return matches
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)

# The class of each STLINK
class STLINK():
    #--- shared variables ---
    _config: ClassVar[ST_CONFIG]
    _class_initialized: ClassVar[bool] = False

    @classmethod
    def initialize_shared(cls, value: ST_CONFIG):
        if not cls._class_initialized:
            cls._config = value
            cls._class_initialized = True

    #--- constructor ---
    def __init__(self, SN):
        self.SN = SN
        self.current_state = ST_STATUS.NO_DEVICE
        self.pause_event = threading.Event()
        self.pause_event.set()

    #--- state relative ---
    def state_move_to_next(self) -> None:
        state_val = self.current_state.value
        # will stuck at FINISHED
        shift = 0 if (state_val == ST_STATUS.FINISHED.value) else 1
        self.current_state = ST_STATUS(state_val + shift)

    def state_reset(self) -> None:
        self.current_state = ST_STATUS.NO_DEVICE

    #--- commands to STM32CubeProgrammer, !! SN related !! ---
    def gen_connection_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187
        cmd = self._config.gen_bin_path_command() + f' -c port={self._config.STLINK_PORT} SN={self.SN}'
        return cmd

    def gen_upload_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -w "C:\Users\Arthur\Desktop\test\fw.elf"
        cmd = self.gen_connection_command() + f' -w "{self._config.FW_ELF_PATH}"'
        return cmd

    def gen_upload_verify_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -v
        cmd = self.gen_upload_command() + ' -v'
        return cmd

    def gen_trigger_exec_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -s 0x08000000
        cmd = self.gen_connection_command() + ' -s 0x08000000'
        return cmd

    #--- board operate by status ---
    ## Use connect to check board is exist or not, but it will trigger exec too
    def check_board(self) -> bool:
        is_connceted = None
        cmd = self.gen_connection_command()

        # board check
        ## send cmd to the board
        out, err = run_command(cmd)

        # stderr check
        if not err:
            # no stderr, only have stdout
            ## parse the return val
            if "Error" in out:
                ## Two type of error in stdout
                ### No stlink -- Error: No debug probe detected.
                # if "No debug probe detected" in out:
                    # raise Exception("No debug probe detected")
                ### No stm32 -- Error: No STM32 target found!
                # if "No STM32 target found" in out:
                    # raise Exception("No STM32 target found - check board connection")
                is_connceted = False
            else :
                is_connceted = True
            return is_connceted
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)
            # TODO: re-initialize the ST-Link

    def upload(self) -> None:
        cmd = self.gen_upload_command()

        # trigger exec
        ## send cmd to the board
        out, err = run_command(cmd)  # out : Start operation achieved successfully

        # stderr check
        if not err:
            ## parse the return val
            if "Error" in out:
                raise Exception("Error when uploading")
            elif "File download complete" in out:
                print("-- File download complete")
            else :
                raise Exception("Unknown situation when uploading")
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)

    def upload_n_verify(self):
        cmd = self.gen_upload_verify_command()
        def state_move_to_next_callback():
            self.state_move_to_next()
        # def _callback

        # trigger upload
        ## send cmd to the board
        out, err = run_command(cmd)
        #### ---------------------------------
        #### out :
        #### File download complete
        #### Time elapsed during download operation: 00:00:01.440
        #### Verifying ...
        #### Download verified successfully
        #### ---------------------------------

        # stderr check
        if not err:
            ## parse the return val

            # before calling verify, change UPLOADING to VERIFYING
            self.state_move_to_next()
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)

    def trigger_exec(self):
        cmd = self.gen_trigger_exec_command()
        # trigger exec
        ## send cmd to the board
        out, err = run_command(cmd)  # out : Start operation achieved successfully

        # stderr check
        if not err:
            ## parse the return val
            if "Error" in out:
                raise Exception("Error when trigger_exec")
            ### success msg -- "Start operation achieved successfully"
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)


    def do_next(self, is_need_verify=False) -> None:
        """
        Will trigger
        1. board check
        2. action start
        3. state move
        """

        # check board still conncted or not at each call
        is_connceted = self.check_board()

        # specify global variables
        global flag_HTTPServerConnnectionError
        global PerBoardSecret
        global PrivKey
        global flag_FwElfNotFound

        # update state (if needed)
        if not is_connceted:
            self.state_reset()
        else:
            if self.current_state == ST_STATUS.NO_DEVICE:
                pass
            elif self.current_state == ST_STATUS.UPLOADING:

                # Modify fw.elf with random PerBoardData and PerBoardSecret
                try:
                    PerBoardSecret, PrivKey = ReplaceELF.modify_fw_elf()
                    flag_FwElfNotFound = False

                    # Record PerBoardSecret
                    if len(PerBoardSecret) == 16 :
                        print(f"PerBoardSecret = {PerBoardSecret}")
                    else:
                        raise ValueError("Invalid PerBoardSecret Array Length")
                    if len(PrivKey) == 7 :
                        print(f"PrivKey = {PrivKey}")
                    else:
                        raise ValueError("Invalid PrivKey Array Length")

                except FileNotFoundError as f:
                    flag_FwElfNotFound = True
                    print("Cannot find fw.elf in this folder")
                    self.current_state = ST_STATUS.NO_DEVICE

                if is_need_verify:
                    self.upload_n_verify()
                else:
                    self.upload()

            elif self.current_state == ST_STATUS.VERIFYING:
                if is_need_verify:
                    print("-- verifying")
                else:
                    print("-- skip verify")

            elif self.current_state == ST_STATUS.TRIGGER_EXEC:
                # Post PerBoardSecret to Cloud Server
                print("FW download verified, log PerBoardData to Server")
                print(f"PerBoardSecret = {PerBoardSecret}")
                print(f"PrivKey = {PrivKey}")
                response = ReplaceELF.http_post_data(post_url, PerBoardSecret, PrivKey)
                if response == 200:
                    print("HTTP POST Success!")
                    flag_HTTPServerConnnectionError = False
                else:
                    print(f"HTTP POST Error: {response}")
                    flag_HTTPServerConnnectionError = True
                    #raise ValueError("Failed to POST PerBoardSecret to Server")

                self.trigger_exec()


            elif self.current_state == ST_STATUS.FINISHED:
                pass
            self.state_move_to_next()


    def bg_daemon(self, stop_event) -> None:
        """
        The only function for thread creation.

        Will auto call do_next() then sleep for "setting.THREAD_SLEEP_INTERVAL" sec.

        Args:
            stop_event : threading.Event
        """
        print("Thread created -- SN : " + str(self.SN))
        while not stop_event.is_set():
            # trigger every device do next, then repeat again and again
            print("\nSN : " + str(self.SN))
            print("State : " + str(self.current_state))
            self.do_next(is_need_verify=True)
            time.sleep(THREAD_SLEEP_INTERVAL)
        print("Thread stop -- SN : " + str(self.SN))

if __name__ == "__main__":

    """ ============ Basic feature test (one device) ================ """

    # import click
    # import threading
    #------- this part can be changed by frontend ----------
    FW_ELF_PATH = 'C:\\Users\\a8701\\Documents\\Development\\hitcon-pcb-badge\\pcb-util\\fw.elf'
    ST_PRO_PATH = 'C:\\Program Files (x86)\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin'
    ST_PRO_EXE = 'STM32_Programmer_CLI.exe'
    #------- this part can be changed by frontend ----------

    # setup shared value to class
    shared_info = ST_CONFIG(
        ELF_PATH=FW_ELF_PATH,
        ST_PRO_PATH=ST_PRO_PATH,
        ST_PRO_EXE=ST_PRO_EXE
        )
    STLINK.initialize_shared(shared_info)

    # test command print (sn unrelated)
    # print("[gen_stlink_list_command]\n -- " + shared_info.gen_stlink_list_command() + "\n")


    # init all object, depend on how many STLINK(SN) we have
    stlink_sn_list = shared_info.list_stlink()
    st_obj = []
    for STLINK_SN in stlink_sn_list:
        st_obj.append(STLINK(STLINK_SN))

    # test command print (sn related)
    if len(st_obj) > 0:
        st_device = st_obj[0]
        # print("[gen_board_check_command]\n -- " + st_device.gen_connection_command() + "\n")
        # print("[gen_upload_command]\n -- " + st_device.gen_upload_command() + "\n")
        # print("[gen_upload_verify_command]\n -- " + st_device.gen_upload_verify_command() + "\n")
        # print("[gen_trigger_exec_command]\n -- " + st_device.gen_trigger_exec_command() + "\n")
        while(1):
            # trigger every device do next, then repeat again and again
            print("\nSN : " + str(st_device.SN))
            print("State : " + str(st_device.current_state))
            print(f"flag_HTTPServerConnnectionError = {flag_HTTPServerConnnectionError}")
            if st_device.current_state is ST_STATUS.FINISHED:
                print("-- All finished !!")
                break
            st_device.do_next()
            time.sleep(2)

    else:
        print("!! No stlink !!")

