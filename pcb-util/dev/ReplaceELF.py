import shutil
import numpy as np
import config
from typing import IO, Self
import tempfile
import contextlib
import os

# Configure the array to find in ELF
old_PerBoardRandom = bytes([
    0xf1, 0xca, 0x4e, 0xa0, 0x48, 0x2f, 0x27, 0x4d,
    0x3d, 0xc2, 0x9c, 0x8c, 0xec, 0x36, 0x83, 0x49
])

old_PubKeyCert = bytes([
    0x7d, 0xf0, 0xde, 0x4c, 0xe2, 0x23, 0x19,
    0xf6, 0xb4, 0xfa, 0xbe, 0x12, 0x6d, 0x41
])

old_PrivKey = bytes([
    0x80, 0x02, 0xb6, 0x03, 0x60, 0xc6, 0x2f
])

class FileReplacer:
    def __init__(self, file: IO[bytes]):
        self.file = file

    def find(self, pattern: bytes, start: int=0) -> int:
        chunk_size = 1024
        last_chunk = b''

        self.file.seek(start)

        while True:
            chunk = self.file.read(chunk_size)
            if not chunk:
                raise ValueError("substring not found")

            try:
                idx = (last_chunk + chunk).index(pattern)
                return self.file.tell() - len(chunk) - len(last_chunk) + idx
            except ValueError:
                pass
            
            last_chunk = chunk

    def replace(self, old: bytes, new: bytes) -> Self:
        start = 0
        while True:
            try:
                start = self.find(old, start)
                self.file.seek(start)
                self.file.write(new)
                start += 1
            except ValueError:
                return self

def duplicate_elf_file(original_file_path) -> IO[bytes]:
    """Duplicate the ELF file."""

    new_file = tempfile.NamedTemporaryFile('w+b', suffix='.elf', delete=False)
    shutil.copyfileobj(open(original_file_path, 'rb'), new_file)
    return new_file

@contextlib.contextmanager
def modify_fw_elf(PrivKey: bytes, PubKeyCert: bytes):
    modded_elf = duplicate_elf_file(config.FW_ELF_PATH)
    try:
        PerBoardRandom = bytes(np.random.randint(0, 256, size=16, dtype=np.uint8))

        # Duplicate the ELF file
        (FileReplacer(modded_elf)
            .replace(old_PerBoardRandom, PerBoardRandom)
            .replace(old_PubKeyCert, PubKeyCert)
            .replace(old_PrivKey, PrivKey))

        modded_elf.close()

        yield modded_elf
    finally:
        os.unlink(modded_elf.name)