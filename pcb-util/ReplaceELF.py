import shutil
from elftools.elf.elffile import ELFFile
import numpy as np


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
            print("Array not found in the ELF file.")
            return

        # Seek to the offset and replace the array
        f.seek(array_offset)
        f.write(bytes(replace_array))
        print(f"Replace array at offset: {hex(array_offset)}")

# Find and Replace the array of PerBoardRandom
def search_and_reaplce_array(elf_file_path, search_array, replace_array):
    array_offset, chunk = find_array_in_elf(elf_file_path, search_array)
    if array_offset is not None:
        print(f"Array found at offset: {hex(array_offset)}")
        print_hex_around_target_array(elf_file_path, array_offset, search_array)
        
        # Replace the array
        replace_array_in_elf(elf_file_path, search_array, replace_array)

        # Find the replaced array again to verify the replacement
        array_offset, chunk = find_array_in_elf(elf_file_path, replace_array)
        if array_offset is not None:
            print(f"Array replaced at offset: {hex(array_offset)}")
            print_hex_around_target_array(elf_file_path, array_offset, replace_array)
        else:
            print("Array failed to be replaced in the ELF file.")
    else:
        print("Array not found in the ELF file.")

def print_array_in_hex(array):
    hex_array = [f'0x{value:02x}' for value in array]
    print("Generated random uint8 array in HEX:", hex_array)
    

if __name__ == "__main__":

    original_elf_path = 'fw.elf'  # Original ELF file path
    MOD_elf_path = 'fwMOD.elf'  # New duplicated ELF file path

    # Configure the array to find in ELF
    search_array_PerBoardRandom = [
            0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d,
            0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49
        ]  # Array to find for PerBoardRandom
        
    search_array_PerBoardSecret = [
            0x13, 0xac, 0x76, 0xfc, 0x1a, 0xa7, 0x0f, 0x92,
            0x05, 0x31, 0x1d, 0xa6, 0x28, 0x4c, 0x8e, 0x94
        ]  # Array to find for PerBoardSecret
    
    # Generate Random array
    replace_array_PerBoardRandom = np.random.randint(0, 256, size=16, dtype=np.uint8)
    replace_array_PerBoardSecret = np.random.randint(0, 256, size=16, dtype=np.uint8)
    
    # Print random array
    print_array_in_hex(replace_array_PerBoardRandom)
    print_array_in_hex(replace_array_PerBoardSecret)
    
    # TODO: Record PerBoardSecret to Cloud
    
    print("\n TODO: Upload PerBoardSecret to Cloud \n")

    # Duplicate the ELF file
    duplicate_elf_file(original_elf_path, MOD_elf_path)

    print("\n Operating fwMOD.elf...... \n")

    # find and replace PerBoardRandom array
    search_and_reaplce_array(MOD_elf_path, search_array_PerBoardRandom, replace_array_PerBoardRandom)
    
    # find and replace PerBoardSecret array
    search_and_reaplce_array(MOD_elf_path, search_array_PerBoardSecret, replace_array_PerBoardSecret)
    
    print("\n Operation Done \n")    
