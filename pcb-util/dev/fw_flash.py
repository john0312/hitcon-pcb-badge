# -*- coding: utf-8 -*-
"""
Created on Sun Jul  7 19:07:46 2024

@author: Arthur (Tora0615)
"""

import threading
import time
from enum import Enum, auto
import ReplaceELF
import config
from utils import run_command
import pcb_logger
from typing import IO
import ecc

# Global
flag_HTTPServerConnnectionError = False
flag_FwElfNotFound = False

# status enum class
class ST_STATUS(Enum):
    ERROR = -1
    NO_DEVICE = 0
    ERASING = auto()
    UPLOADING = auto()
    TRIGGER_EXEC = auto()
    FINISHED = auto()

# The class of each STLINK
class STLINK():
    #--- constructor ---
    def __init__(self, SN):
        self.SN = SN
        self.current_state = ST_STATUS.NO_DEVICE
        self.pause_event = threading.Event()
        self.pause_event.set()

    #--- state relative ---
    def state_move_to_next(self) -> None:
        terminal_states = ST_STATUS.FINISHED, ST_STATUS.ERROR
        shift = 0 if self.current_state in terminal_states else 1
        self.current_state = ST_STATUS(self.current_state.value + shift)

    def state_reset(self) -> None:
        self.current_state = ST_STATUS.NO_DEVICE

    #--- board operate by status ---
    ## Use connect to check board is exist or not, but it will trigger exec too
    def check_board(self) -> bool:
        is_connceted = None
        cmd = config.st_config.gen_connection_command(self.SN)

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
            self.current_state = ST_STATUS.ERROR
            return False

    def erase(self) -> None:
        cmd = config.st_config.gen_erase_command(self.SN)

        out, err = run_command(cmd)

        if 'Mass erase successfully achieved' not in out:
            self.current_state = ST_STATUS.ERROR
            raise ValueError(err)

    def upload_n_verify(self, elf_file: IO[bytes]):
        cmd = config.st_config.gen_upload_verify_command(self.SN, elf_file.name)

        # trigger upload
        ## send cmd to the board
        out, err = run_command(cmd)

        if err or 'error' in out.lower():
            self.current_state = ST_STATUS.ERROR
            raise ValueError(out, err)
        else:
            ## parse the return val
            self.state_move_to_next()

    def trigger_exec(self):
        cmd = config.st_config.gen_trigger_exec_command(self.SN)
        # trigger exec
        ## send cmd to the board
        out, err = run_command(cmd)  # out : Start operation achieved successfully

        if err or 'error' in out.lower():
            self.current_state = ST_STATUS.ERROR
            raise ValueError(out, err)
        else:
            ## parse the return val
            self.state_move_to_next()

    def do_next(self) -> None:
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
        global PrivKey
        global flag_FwElfNotFound

        # update state (if needed)
        if not is_connceted:
            self.state_reset()
        else:
            if self.current_state == ST_STATUS.ERROR:
                pass
            if self.current_state == ST_STATUS.NO_DEVICE:
                pass
            elif self.current_state == ST_STATUS.ERASING:
                self.erase()

            elif self.current_state == ST_STATUS.UPLOADING:

                while True:
                    try:
                        PrivKey = ecc.gen_key(config.TEAM == 'RED')
                        PubKeyCert = pcb_logger.post_board_data(PrivKey)
                        break
                    except pcb_logger.PrivKeyExistsException:
                        pass

                # Modify fw.elf with the private key
                try:
                    flag_FwElfNotFound = False
                    with ReplaceELF.modify_fw_elf(PrivKey, PubKeyCert) as elf_file:
                        self.upload_n_verify(elf_file)

                except FileNotFoundError as f:
                    flag_FwElfNotFound = True
                    self.current_state = ST_STATUS.NO_DEVICE
                    raise f

                pcb_logger.post_commit_privkey(PrivKey)

            elif self.current_state == ST_STATUS.TRIGGER_EXEC:
                self.trigger_exec()

            elif self.current_state == ST_STATUS.FINISHED:
                pass

            self.state_move_to_next()


    def bg_daemon(self, stop_event: threading.Event) -> None:
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
            self.do_next()
            time.sleep(config.THREAD_SLEEP_INTERVAL)
        print("Thread stop -- SN : " + str(self.SN))

def list_stlink() -> list:
    import re
    out, err = run_command(config.st_config.gen_stlink_list_command())

    if not err:
        # regex
        pattern = r"ST-LINK SN\s+:\s+(\S+)"
        matches = re.findall(pattern, out)

        return matches
    else:
        # input command error, e.g. xxx not recognized as an internal or external command
        raise ValueError(err)

if __name__ == '__main__':
    stlinks = list_stlink()
    assert len(stlinks) == 1
    stlink = STLINK(stlinks[0])
    import threading
    stop_event = threading.Event()
    try:
        stlink.bg_daemon(stop_event)
    except Exception as e:
        stop_event.set()
        raise e