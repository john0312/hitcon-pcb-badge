


FW_ELF_PATH = 'fw.elf'
ST_PRO_PATH = r'C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe'
POST_URL = 'https://pcb-log.hitcon2025.online/'
CLI_QUIT_SCAN_INTERVAL = 0.1
THREAD_SLEEP_INTERVAL = 2
MAX_ST_QTY = 10
CURSES_RESERVE_LINE = 1

TEAM = 'BLUE'

def set_team(is_blue: bool):
    global TEAM
    TEAM = 'BLUE' if is_blue else 'RED'

class ST_CONFIG:
    def __init__(self, PORT='SWD'):
        self.STLINK_PORT = PORT
        self.FW_ELF_PATH = FW_ELF_PATH
        self.ST_PRO_PATH = ST_PRO_PATH

    #--- commands to STM32CubeProgrammer, sn unrelated ---
    def gen_bin_path_command(self) -> str:
        cmd = f'"{self.ST_PRO_PATH}"'
        return cmd

    def gen_stlink_list_command(self) -> str:
        cmd = f'{self.gen_bin_path_command()} -l'
        return cmd
    
    def gen_connection_command(self, SN: str) -> str:
        cmd = f'{self.gen_bin_path_command()} -c port={self.STLINK_PORT} SN={SN}'
        return cmd

    def gen_upload_command(self, SN: str) -> str:
        cmd = f'{self.gen_connection_command(SN)} -w "{self.FW_ELF_PATH}"'
        return cmd

    def gen_upload_verify_command(self, SN: str) -> str:
        cmd = f'{self.gen_upload_command(SN)} -v'
        return cmd
    
    def gen_trigger_exec_command(self, SN: str) -> str:
        cmd = f'{self.gen_connection_command(SN)} -s 0x08000000'
        return cmd

st_config = ST_CONFIG()
