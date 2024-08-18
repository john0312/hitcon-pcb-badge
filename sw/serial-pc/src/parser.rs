use std::collections::VecDeque;

use crate::crc::{Crc, CrcBuilder};

const HEADER_SZ: usize = 16;
const PREAMBLE: [u8; 8] = [0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5];
pub const PING_TYPE: u8 = 0xD0;

pub struct PacketParser {
    buffer: VecDeque<u8>,
    crc: Crc,
}

#[derive(Debug)]
pub struct Packet {
    pub typ: u8,
    pub data: Vec<u8>,
}

impl PacketParser {
    pub fn new() -> Self {
        PacketParser {
            buffer: VecDeque::new(),
            crc: CrcBuilder::default().build(),
        }
    }

    pub fn extend(&mut self, data: &[u8]) {
        self.buffer.extend(data);
    }

    pub fn parse(&mut self) -> Vec<Packet> {
        let mut pkts = vec![];
        while self.buffer.len() >= HEADER_SZ {
            while let Some(byte) = self.buffer.front() {
                match byte {
                    0x55 => break,
                    _ => {
                        self.buffer.pop_front();
                    }
                }
            }
            if self.buffer.len() < HEADER_SZ {
                return vec![];
            }
            if !self.buffer.range(0..8).eq(PREAMBLE.iter()) {
                self.buffer.pop_front();
                continue;
            }
            let data_len = self.buffer[10];
            if self.buffer.len() < HEADER_SZ + (data_len as usize) {
                break;
            }
            let mut pkt_bytes: Vec<u8> = self
                .buffer
                .range(0..HEADER_SZ + (data_len as usize))
                .copied()
                .collect();
            for i in 12..16 {
                pkt_bytes[i] = 0
            }
            match pkt_bytes.len() % 4 {
                1 => {
                    pkt_bytes.extend([0, 0, 0]);
                }
                2 => {
                    pkt_bytes.extend([0, 0]);
                }
                3 => {
                    pkt_bytes.extend([0]);
                }
                _ => (),
            }
            let check_calculate = self.crc.hash_bytes(&pkt_bytes);
            if !check_calculate.iter().eq(self.buffer.range(12..16)) {
                self.buffer.drain(0..8);
                continue;
            }

            // valid
            self.buffer.drain(0..HEADER_SZ + (data_len as usize));
            let pkt_type = pkt_bytes[11];
            pkts.push(Packet {
                typ: pkt_type,
                data: pkt_bytes[HEADER_SZ..HEADER_SZ + (data_len as usize)].to_vec(),
            });
        }
        pkts
    }
}
