# (Under Construction)
# This script provide the following function:
# 1. Search for the target array in the ELF file
# 2. Print the HEX dump around the target array

from elftools.elf.elffile import ELFFile

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

            # Search for the array in the current chunk
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

def print_hex_around_array(elf_file_path, array_offset, search_array, context_size=16):
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

# Example usage
search_array = [
    0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d,
    0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49
]  # Updated search array
elf_file_path = 'fw.elf'  # Updated ELF file path

array_offset, chunk = find_array_in_elf(elf_file_path, search_array)
if array_offset is not None:
    print(f"Array found at offset: {hex(array_offset)}")
    print_hex_around_array(elf_file_path, array_offset, search_array)
else:
    print("Array not found in the ELF file.")
