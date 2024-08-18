// https://www.reddit.com/r/embedded/comments/13xiz1u/stm32_crc32_incompatible_with_all_other_existing/
pub struct CrcBuilder {
    poly: u32,
}

impl Default for CrcBuilder {
    fn default() -> Self {
        CrcBuilder::new(0x04C11DB7)
    }
}

impl CrcBuilder {
    pub fn new(poly: u32) -> Self {
        CrcBuilder { poly }
    }
    pub fn build(&self) -> Crc {
        let mut table = [0_u32; 256];
        for i in 0_u32..256 {
            let mut c = i << 24;
            for _ in 0..8 {
                // c = (c << 1) ^ self.poly;
                c = if (c & 0x80000000) != 0 {
                    (c << 1) ^ self.poly
                } else {
                    c << 1
                };
            }
            table[i as usize] = c;
        }
        Crc { table }
    }
}

pub struct Crc {
    table: [u32; 256],
}

impl Default for Crc {
    fn default() -> Self {
        CrcBuilder::default().build()
    }
}

impl Crc {
    pub fn hash<T: AsRef<[u8]>>(&self, data: T) -> u32 {
        let data = data.as_ref();
        let mut _crc = 0xFFFFFFFF_u32;
        let mut i = 0_usize;
        while i < data.len() {
            let bytes = [data[i + 3], data[i + 2], data[i + 1], data[i]];
            i += 4;
            for byte in bytes {
                _crc = (_crc << 8) ^ self.table[((_crc >> 24) ^ (byte as u32)) as usize];
            }
        }
        _crc
    }
    pub fn hash_bytes<T: AsRef<[u8]>>(&self, data: T) -> [u8; 4] {
        let hash = self.hash(data);
        hash.to_le_bytes()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test() {
        let c = CrcBuilder::new(0x04C11DB7).build();
        let data = b"\x55\x55\x55\x55\x55\x55\x55\xD5\x00\x00\x00\xD0\0\0\0\0";
        let hash = [0x0C, 0xFC, 0x48, 0xFD];
        assert_eq!(c.hash_bytes(data), hash);
    }
}
