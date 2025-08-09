# https://www.reddit.com/r/embedded/comments/13xiz1u/stm32_crc32_incompatible_with_all_other_existing/
# CRC32 class
class Crc32:
    crc_table = {}

    def __init__(self, _poly):
        # Generate CRC table for polynomial
        for i in range(256):
            c = i << 24
            for j in range(8):
                c = (c << 1) ^ _poly if (c & 0x80000000) else c << 1
            self.crc_table[i] = c & 0xFFFFFFFF

    # Calculate CRC from input buffer
    def calculate(self, buf):
        crc = 0xFFFFFFFF

        i = 0
        while i < len(buf):
            b = [buf[i+3], buf[i+2], buf[i+1], buf[i+0]]
            i += 4
            for byte in b:
                crc = ((crc << 8) & 0xFFFFFFFF) ^ self.crc_table[(crc >> 24) ^ byte]
        return crc

    # Create bytes array from integer input
    def crc_int_to_bytes(self, i):
        return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]

# Use it
# Prepare CRC block
# crc = Crc32(0x04C11DB7)
# result = crc.calculate(b'\x55\x55\x55\x55\x55\x55\x55\xD5\x00\x00\x00\xD0\0\0\0\0')
# print(hex(result))