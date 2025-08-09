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

# Global
flag_HTTPServerConnnectionError = False
flag_FwElfNotFound = False
PerBoardSecret = b''
PrivKey: bytes = b''

# status enum class
class ST_STATUS(Enum):
    NO_DEVICE = 0
    UPLOADING = auto()
    VERIFYING = auto()
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
        state_val = self.current_state.value
        # will stuck at FINISHED
        shift = 0 if (state_val == ST_STATUS.FINISHED.value) else 1
        self.current_state = ST_STATUS(state_val + shift)

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
            raise ValueError(err)
            # TODO: re-initialize the ST-Link

    def upload(self) -> None:
        cmd = config.st_config.gen_upload_command(self.SN)

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
        cmd = config.st_config.gen_upload_verify_command(self.SN)

        # trigger upload
        ## send cmd to the board
        out, err = run_command(cmd)

        # stderr check
        if not err:
            ## parse the return val

            # before calling verify, change UPLOADING to VERIFYING
            self.state_move_to_next()
        else:
            # input command error, e.g. xxx not recognized as an internal or external command
            raise ValueError(err)

    def trigger_exec(self):
        cmd = config.st_config.gen_trigger_exec_command(self.SN)
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
                pcb_logger.post_commit_privkey(PerBoardSecret, PrivKey)

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
            time.sleep(config.THREAD_SLEEP_INTERVAL)
        print("Thread stop -- SN : " + str(self.SN))