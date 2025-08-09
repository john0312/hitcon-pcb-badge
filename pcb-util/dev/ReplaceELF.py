import base64
import shutil
from elftools.elf.elffile import ELFFile
import numpy as np
import requests
import configparser
import os
import json
import ecc

original_elf_path = 'fw.elf'  # Original ELF file path
MOD_elf_path = 'fwMOD.elf'  # New duplicated ELF file path

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
#STLINK_AUTO_DETECTION_INTERVAL = config.getint('Settings', 'STLINK_AUTO_DETECTION_INTERVAL')
EN_PCB_LOG = config.getint('HTTP', 'EN_PCB_LOG')
SERVER_PRIV_KEY = int(config.get('GAME', 'SERVER_PRIV_KEY'))
SERVER_PUB_KEY = base64.b64decode(config.get('GAME', 'SERVER_PUB_KEY'))
TEAM = config.get('GAME', 'TEAM')

if TEAM not in ['BLUE', 'RED']:
    raise ValueError("Invalid TEAM value in config.ini. Must be 'BLUE' or 'RED'.")

# Verify the server priv key
def verify_server_priv_key():
    curve, G, order = ecc.mysecp()
    pubkey = (SERVER_PRIV_KEY * G).compact()
    if pubkey != SERVER_PUB_KEY:
        raise ValueError("Invalid SERVER_PRIV_KEY or SERVER_PUB_KEY in config.ini.")

verify_server_priv_key()

# Configure the array to find in ELF
search_array_PerBoardRandom = [
        0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d,
        0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49
    ]  # Array to find for PerBoardRandom

search_array_PerBoardSecret = [
        0x13, 0xac, 0x76, 0xfc, 0x1a, 0xa7, 0x0f, 0x92,
        0x05, 0x31, 0x1d, 0xa6, 0x28, 0x4c, 0x8e, 0x94
    ]  # Array to find for PerBoardSecret

search_array_PubKeyCert = [
    0x7d, 0xf0, 0xde, 0x4c, 0xe2, 0x23, 0x19,
    0xf6, 0xb4, 0xfa, 0xbe, 0x12, 0x6d, 0x41
]

search_array_PrivKey = [
    0x80, 0x02, 0xb6, 0x03, 0x60, 0xc6, 0x2f
]

def duplicate_elf_file(original_file_path, new_file_path):
    """Duplicate the ELF file."""
    shutil.copy(original_file_path, new_file_path)
    print(f"Duplicated {original_file_path} to {new_file_path}")

# Print the hex content around the specified position
def print_hex_around_target_array(elf_file_path, array_offset, search_array, context_size=16):
    with open(elf_file_path, 'rb') as f:
        # Read the entire ELF file data
        f.seek(0, 2)  # Move to the end of the file
        file_size = f.tell()
        f.seek(0)  # Move back to the start of the file

        # Calculate the start and end offsets for context
        start_offset = max(0, array_offset - context_size)
        end_offset = min(file_size, array_offset + len(search_array) + context_size)

        # Read the relevant section of the ELF file
        f.seek(start_offset)
        relevant_data = f.read(end_offset - start_offset)

        # Print the hexadecimal representation of the relevant section
        print(f"Hexadecimal Data around the found array at offset {hex(array_offset)}:")
        hex_lines = []
        for i in range(0, len(relevant_data), 16):
            line_bytes = relevant_data[i:i+16]
            hex_str = ' '.join(f'{byte:02x}' for byte in line_bytes)
            offset_str = f'{start_offset + i:08x}'
            hex_lines.append(f"{offset_str}: {hex_str}")

        # Print all lines
        print('\n'.join(hex_lines))

def find_array_in_elf(elf_file_path, search_array):
    with open(elf_file_path, 'rb') as f:
        elf = ELFFile(f)

        # Set the chunk size (e.g., 1 MB)
        chunk_size = 1024 * 1024

        offset = 0
        while True:
            # Read the next chunk
            f.seek(offset)
            chunk = f.read(chunk_size)
            if not chunk:
                break

            # find for the array in the current chunk
            array_bytes = bytes(search_array)
            index = chunk.find(array_bytes)
            if index != -1:
                # Array found, calculate the absolute offset
                absolute_offset = offset + index
                return absolute_offset, chunk  # Return the chunk where the array was found

            # Update the offset for the next chunk
            offset += chunk_size

    # Array not found
    return None, None

def replace_array_in_elf(elf_file_path, search_array, replace_array):
    if len(search_array) != len(replace_array):
        raise ValueError("find and replace arrays must be of the same length.")

    with open(elf_file_path, 'r+b') as f:
        array_offset, _ = find_array_in_elf(elf_file_path, search_array)
        if array_offset is None:
            raise ValueError("Array not found in the ELF file.")

        # Seek to the offset and replace the array
        f.seek(array_offset)
        f.write(bytes(replace_array))
        #print(f"Replace array at offset: {hex(array_offset)}")

    return array_offset

# Find and Replace the array of PerBoardRandom
def search_and_reaplce_array(elf_file_path, search_array, replace_array):
    array_offset, chunk = find_array_in_elf(elf_file_path, search_array)
    if array_offset is not None:
        print(f"Array found at offset: {hex(array_offset)}")
        #print_hex_around_target_array(elf_file_path, array_offset, search_array)

        # Replace the array
        err = replace_array_in_elf(elf_file_path, search_array, replace_array)

        # Find the replaced array again to verify the replacement
        array_offset, chunk = find_array_in_elf(elf_file_path, replace_array)
        if array_offset is not None:
            print(f"Array replaced at offset: {hex(array_offset)}")
            #print_hex_around_target_array(elf_file_path, array_offset, replace_array)
        else:
            raise ValueError("Array failed to be replaced in the ELF file.")
        return array_offset
    else:
        raise ValueError("Array not found in the ELF file.")

def print_array_in_hex(array):
    hex_array = [f'0x{value:02x}' for value in array]
    print("Generated random uint8 array in HEX:", hex_array)

def http_post_data(url, per_board_secret, priv_key):
    per_board_secret = base64.b64encode(per_board_secret).decode('utf-8')
    priv_key = base64.b64encode(priv_key).decode('utf-8')
    data = {
        "board_secret": per_board_secret,
        "priv_key": priv_key
    }
    data = json.dumps(data)
    response = requests.post(url, data)

    return response.status_code

def modify_fw_elf():
    replace_array_PerBoardRandom = np.random.randint(0, 256, size=16, dtype=np.uint8)
    replace_array_PerBoardSecret = np.random.randint(0, 256, size=16, dtype=np.uint8)

    replace_array_PrivKey, replace_array_PubKeyCert = ecc.gen_key(SERVER_PRIV_KEY, TEAM == 'RED')

    # Duplicate the ELF file
    duplicate_elf_file(original_elf_path, MOD_elf_path)
    # Modify array in ELF
    array_offset_PerBoardRandom = search_and_reaplce_array(MOD_elf_path, search_array_PerBoardRandom, replace_array_PerBoardRandom)
    array_offset_PerBoardSecret = search_and_reaplce_array(MOD_elf_path, search_array_PerBoardSecret, replace_array_PerBoardSecret)
    array_offset_PubKeyCert = search_and_reaplce_array(MOD_elf_path, search_array_PubKeyCert, replace_array_PubKeyCert)
    array_offset_PrivKey = search_and_reaplce_array(MOD_elf_path, search_array_PrivKey, replace_array_PrivKey)

    return replace_array_PerBoardSecret, replace_array_PrivKey

if __name__ == "__main__":
    replace_array_PerBoardSecret, replace_array_PrivKey = modify_fw_elf()

    # TODO: Test posting PerBoardSecret to Cloud
    print(f"\n POST PerBoardSecret to {post_url}\n")

    response = http_post_data(post_url, replace_array_PerBoardSecret, replace_array_PrivKey)
    if response == 200:
        print("HTTP POST Success!")
    else:
        print(f"Error: {response}")

    print("\n Operation Done \n")
