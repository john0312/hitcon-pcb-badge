use clap::Parser;
use crc::Crc;
use parser::{PacketParser, PING_TYPE};
use rayon::prelude::*;
use sha3::{Digest, Sha3_256};
use std::{
    cmp::Ordering,
    io::{self, Read, Write},
    net::{TcpListener, TcpStream},
    thread,
    time::Duration,
};

mod crc;
mod parser;

/// connect to serial
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct ServerArgs {
    /// serial device name
    #[arg(short, long)]
    dev: Option<String>,
    /// dump column data and calculate score
    #[arg(long)]
    dump: bool,
}

/// Send packet to serial with specific type
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct ClientArgs {
    /// Send packet type
    #[arg(short, long)]
    pkt_type: u8,
}

fn main() {
    match TcpListener::bind("127.0.0.1:7878") {
        Ok(listener) => {
            println!("server mode");
            server(listener);
        }
        Err(e) => match e.kind() {
            io::ErrorKind::AddrInUse => {
                println!("client mode");
                client();
            }
            _ => {
                panic!("{:?}", e);
            }
        },
    }
}

fn server(listener: TcpListener) {
    let args = ServerArgs::parse();
    let mut dump = args.dump;

    // baud rate 28800
    let ports = serialport::available_ports().expect("No ports found!");
    for p in &ports {
        println!("{}", p.port_name);
    }
    let devname = match args.dev {
        Some(dev) => dev,
        None => ports[0].port_name.clone(),
    };
    let mut port = serialport::new(devname, 28800)
        .timeout(Duration::from_millis(10))
        .open()
        .expect("Failed to open port");
    let mut p2 = port.try_clone().unwrap();

    // forward socket to serial thread
    thread::spawn(move || {
        for stream in listener.incoming() {
            let mut stream = stream.unwrap();
            let mut buf = [0; 1024];
            let len = stream.read(&mut buf).unwrap();
            let buf = buf[..len].to_vec();
            p2.write_all(&buf).unwrap();
            println!("serial_send(hex): `{:02X?}`", buf);
        }
    });

    let mut packet_parser = PacketParser::new();

    // receive loop
    let mut cells: Vec<Vec<u8>> = vec![];
    loop {
        let mut serial_buf: Vec<u8> = vec![0; 1024];
        match port.read(serial_buf.as_mut_slice()) {
            Ok(t) => {
                println!("serial_recv(hex): {:02X?}", &serial_buf[..t]);
                packet_parser.extend(&serial_buf[..t]);
                let pkts = packet_parser.parse();
                if !pkts.is_empty() && dump {
                    port.write_all(&make_pkt(5)).unwrap();
                    dump = false;
                }
                let pkts = pkts
                    .into_iter()
                    .filter(|p| p.typ != PING_TYPE)
                    .collect::<Vec<_>>();
                for pkt in pkts {
                    match pkt.typ {
                        3 => {
                            println!("type={} data={:02X?}", pkt.typ, pkt.data);
                            cells.push(pkt.data);
                        }
                        _ => {
                            println!("{:?}", pkt);
                        }
                    }
                }
                io::stdout().flush().unwrap();
                match cells.len().cmp(&130) {
                    Ordering::Equal => {
                        break;
                    }
                    Ordering::Greater => {
                        panic!("received too many cells");
                    }
                    Ordering::Less => (),
                }
            }
            Err(ref e) if e.kind() == io::ErrorKind::TimedOut => (),
            Err(e) => eprintln!("{:?}", e),
        }
    }
    let mut dino = 0_u32;
    let mut tetris = 0_u32;
    let mut snake = 0_u32;
    cells.iter().filter(|cell| cell[0] >= 16).for_each(|cell| {
        match cell[0] {
            125 => {
                dino = u32::from_le_bytes(cell[1..5].try_into().unwrap());
            }
            126 => {
                tetris = u32::from_le_bytes(cell[1..5].try_into().unwrap());
                snake = u32::from_le_bytes(cell[5..9].try_into().unwrap());
            }
            _ => (),
        }
    });
    let score: u32 = cells
        .into_par_iter()
        .filter(|cell| cell[0] < 16)
        .map(|cell| {
            let data = b"HITCON\0".iter().copied().chain(cell);
            let data = data.collect::<Vec<_>>();
            let hash = Sha3_256::digest(&data);
            let mut count = 0;
            for byte in hash {
                match byte.leading_zeros() {
                    8 => {
                        count += 8;
                    }
                    x => {
                        count += x;
                        break;
                    }
                }
            }
            count * count
        })
        .sum();
    let score = score / 16 + 1;
    println!("dino {}", dino);
    println!("tetris {}", tetris);
    println!("snake {}", snake);
    println!("score {}", score);
}

fn client() {
    let args = ClientArgs::parse();
    let mut stream =
        TcpStream::connect("127.0.0.1:7878").expect("Couldn't connect to the server...");
    let pkt = make_pkt(args.pkt_type);
    println!("send `{:02X?}`", pkt);
    stream.write_all(&pkt).unwrap();
}

fn make_pkt(pkt_type: u8) -> Vec<u8> {
    // preamble, id, len, type, checksum
    let mut pkt: Vec<_> = b"\x55\x55\x55\x55\x55\x55\x55\xd5".to_vec();
    pkt.extend(b"\0\0");
    pkt.extend(b"\0");
    // pkt.extend(b"\x05");
    pkt.extend([pkt_type]);
    pkt.extend(b"\0\0\0\0");
    let c = Crc::default();
    let checksum = c.hash_bytes(&pkt);
    pkt.drain(pkt.len() - 4..);
    pkt.extend(checksum);
    pkt
}
