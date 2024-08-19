use clap::Parser;
use crc::Crc;
use parser::{PacketParser, PING_TYPE};
use rayon::iter::{IntoParallelRefIterator, ParallelIterator};
use rayon::prelude::*;
use sha3::{Digest, Sha3_256};
use std::{
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
            p2.write(&buf).unwrap();
            println!("serial_send(hex): `{:02X?}`", buf);
        }
    });

    let mut packet_parser = PacketParser::new();

    // receive loop
    let mut cells: Vec<[u8; 8]> = vec![];
    loop {
        let mut serial_buf: Vec<u8> = vec![0; 1024];
        match port.read(serial_buf.as_mut_slice()) {
            Ok(t) => {
                println!("serial_recv(hex): {:02X?}", &serial_buf[..t]);
                // buffer.extend(&serial_buf[..t]);
                packet_parser.extend(&serial_buf[..t]);
                let pkts = packet_parser.parse();
                let pkts = pkts
                    .into_iter()
                    .filter(|p| p.typ != PING_TYPE)
                    .collect::<Vec<_>>();
                // TODO: calculate score
                // cell_score[x][y] = count_prefix_zero(sha3_256(... cell[x][y]))
                // score = sum([cell_score[x][y]*cell_score[x][y] for x in range(cols) for y in range(rows)])//16
                for pkt in pkts {
                    match pkt.typ {
                        3 => {
                            println!("type={} data={:02X?}", pkt.typ, pkt.data);
                            cells.push(pkt.data[1..].try_into().unwrap());
                        }
                        _ => {
                            println!("{:?}", pkt);
                        }
                    }
                }
                io::stdout().flush().unwrap();
                if cells.len() == 130 {
                    break;
                } else if cells.len() > 130 {
                    panic!("received too many cells");
                }
            }
            Err(ref e) if e.kind() == io::ErrorKind::TimedOut => (),
            Err(e) => eprintln!("{:?}", e),
        }
    }
    let score: u32 = cells
        .par_iter()
        .enumerate()
        .filter(|&(i, _)| i <= 128)
        .map(|(index, cell)| {
            let mut data = vec![b'H', b'I', b'T', b'C', b'O', b'N'];
            data.push((((index / 8) & 0xFF00) >> 8).try_into().unwrap());
            data.push(((index / 8) & 0xFF).try_into().unwrap());
            data.extend_from_slice(cell);
            let hash = Sha3_256::digest(data);
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
    println!("scores {:?}", score);
}

fn client() {
    let args = ClientArgs::parse();
    let mut stream =
        TcpStream::connect("127.0.0.1:7878").expect("Couldn't connect to the server...");
    let pkt = make_pkt(args.pkt_type);
    println!("send `{:02X?}`", pkt);
    stream.write(&pkt).unwrap();
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
    return pkt;
}
