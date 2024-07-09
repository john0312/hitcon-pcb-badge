# -*- coding: utf-8 -*-
"""
Created on Sun Jul  7 19:07:46 2024

@author: Arthur
"""

from enum import Enum, auto
import os
import subprocess

# status enum class
class ST_STATUS(Enum):
    NO_DEVICE = 0
    UPLOADING = auto()
    VERIFYING = auto()
    TRIGGER_EXEC = auto()
    FINISHED = auto()
    
# data class
class ST_CONFIG():
    def __init__(self, PORT, ELF_PATH, ST_PRO_PATH, ST_PRO_EXE):
        self.STLINK_PORT = PORT
        self.FW_ELF_PATH = ELF_PATH
        self.ST_PRO_PATH = ST_PRO_PATH
        self.ST_PRO_EXE = ST_PRO_EXE
    
# The class of each STLINK
class STLINK():
    #--- shared variables ---
    _config = None
    @classmethod
    def initialize_shared_var(cls, value):
        if cls._config is None: 
            cls._config = value
    
    #--- constructor ---
    def __init__(self, SN):
        self.SN = SN
        self.current_state = ST_STATUS.NO_DEVICE

    #--- state relative ---
    def state_move_to_next(self) -> None:
        state_val = self.current_state.value
        shift = 0 if (state_val == ST_STATUS.FINISHED.value) else 1
        self.current_state = ST_STATUS(state_val + shift)
        
    def state_reset(self) -> None:
        self.current_state = ST_STATUS.NO_DEVICE

    #--- commands to STM32CubeProgrammer ---
    def basic_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187
        cmd = f"{self._config.ST_PRO_PATH}\\{self._config.ST_PRO_EXE} -c port={self._config.STLINK_PORT} SN={self.SN}"
        return cmd

    def upload_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -w "C:\Users\Arthur\Desktop\test\fw.elf"
        cmd = self.basic_command() + f" -w {self._config.FW_ELF_PATH}"
        return cmd
    
    def verify_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -v
        cmd = self.basic_command() + " -v"
        return cmd
    
    def trigger_exec_command(self) -> str:
        # command : STM32_Programmer_CLI.exe -c port=SWD SN=56FF6F066682495259282187 -s 0x08000000
        cmd = self.basic_command() + " -s 0x08000000"
        return cmd
    
    #--- interface to use STM32CubeProgrammer ---
    def run_command(cmd):
        # prevent " 'C:\Program' is not recognized..." error caused by space
        cmd = cmd.replace('Program Files', '"Program Files"')
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        return stdout, stderr

    #--- ---
    def check_board(self) -> bool:
        is_connceted = None
        cmd = self.basic_command()
        
        # board check
        ## ping the board
        out, err = self.run_command(cmd)
        ## parse the return val
        if "Error" in out:
            ### Two type of error, 
            ### No stlink -- Error: No debug probe detected.
            ### No stm32 -- Error: No STM32 target found!
            is_connceted = False
        else : 
            is_connceted = True
        
        return is_connceted
    
    def upload(self):
        
        pass
    
    def verify(self):
        
        pass
    
    def trigger_exec(self):
        
        pass


    def do_next(self):
        # check board still conncted or not at each call
        is_connceted = self.check_board()
        
        # update state (if needed)
        if not is_connceted:
            self.state_reset()
        else:
            if self.current_state == ST_STATUS.NO_DEVICE:
                pass
            elif self.current_state == ST_STATUS.UPLOADING:
                self.upload()
            elif self.current_state == ST_STATUS.VERIFYING:
                self.verify()
            elif self.current_state == ST_STATUS.TRIGGER_EXEC:
                self.trigger_exec()
            elif self.current_state == ST_STATUS.FINISHED:
                pass
            
            self.state_move_to_next()

if __name__ == "__main__":
    import time
    
    STLINK_SN = "56FF6F066682495259282187"
    STLINK_PORT = "SWD"
    FW_ELF_PATH = "C:\\Users\\Arthur\\Desktop\\test\\fw.elf"
    ST_PRO_PATH = "C:\\Program Files\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin"
    ST_PRO_EXE = "STM32_Programmer_CLI.exe"
    
    # setup shared value to class
    shared_config = ST_CONFIG(
        PORT=STLINK_PORT, 
        ELF_PATH=FW_ELF_PATH, 
        ST_PRO_PATH=ST_PRO_PATH, 
        ST_PRO_EXE=ST_PRO_EXE
        )
    STLINK.initialize_shared_var(shared_config)
    
    # init all object, depend on how many STLINK(SN) we have
    stlink_list = [STLINK(STLINK_SN)]
    
    print(stlink_list[0].basic_command())
    print(stlink_list[0].upload_command())
    print(stlink_list[0].verify_command())
    print(stlink_list[0].trigger_exec_command())
    pass

    while(1):
        for st_device in stlink_list:
            st_device.do_next()
        time.sleep(1000)