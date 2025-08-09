#--- interface to use STM32CubeProgrammer by command---
import subprocess
from typing import Tuple

def run_command(cmd) -> Tuple[str, str]:
    process = subprocess.Popen(
        cmd, shell=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, errors='ignore')
    stdout, stderr = process.communicate()
    return stdout, stderr