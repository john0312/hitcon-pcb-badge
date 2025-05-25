

"""
USB Serial Commands:

Necessary Fields:
Preamble - 8 bytes. 0xD5 0x55 0x55 0x55 0x55 0x55 0x55 0x55

PacketType - 1 byte
    base station to badge:
        * 0x01 for QueueTxBufferRequest
        * 0x81 for QueueTxBufferResponse
        * 0x02 for RetrieveRxBufferRequest 
        * 0x82 for RetrieveRxBufferResponse
        * 0x03 for GetStatusRequest
        * 0x83 for GetStatusResponse

    badge to base station:
        * 0x04 for PushTxBufferRequest
        * 0x84 for PushTxBufferResponse
        * 0x05 for PopRxBufferRequest
        * 0x85 for PopRxBufferResponse
        * 0x06 for SendStatusRequest
        * 0x86 for SendStatusResponse

PacketSequence - 1 byte. A random byte to denote which response maps to which request.
    The response should have the same packet sequence as the request.

PacketSize - 1 byte.

Optional Fields:

Payload - x bytes, if PacketSize not 0x00

Payload content of each packet type:

QueueTxBufferRequest: (Computer to Badge)
x bytes - The entire buffer data to queue for transmit.

QueueTxBufferResponse: (Badge to Computer)
1 byte - 0x01 for buffer full, 0x02 for success.

RetrieveRxBufferRequest: (Computer to Badge)
None

RetrieveRxBufferResponse: (Badge to Computer)
x bytes for the entire buffer received.

GetStatusRequest: (Computer to Badge)
None

GetStatusResponse: (Badge to Computer)
1 byte - 0x01 for failure, 0x02 for success.
1 byte - Status:
            * 0x01: Set for not ready to receive, all received buffers has been populated with received packet.
            * 0x02: Set for ready to receive, there are empty received buffers exist on badge.


PushTxBufferRequest: (Badge to Computer) (for transfer badge status to allow QueueTxBufferRequest to be sent)
None

PushTxBufferResponse: (Computer to Badge)
x bytes - The entire buffer data to queue for transmit.

PopRxBufferRequest: (Badge to Computer)
x bytes - The entire buffer data to queue for transmit.

PopRxBufferResponse: (Computer to Badge)
1 byte - 0x01 for failure, 0x02 for success.

SendStatusRequest: (Badge to Computer)
1 byte - The status, same as above.

SendStatusResponse: (Computer to Badge)
None

"""

import serial
import enum
from config import Config
import asyncio
import time
from dataclasses import dataclass
from typing import Optional
import logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

# PacketType enum
class PT(enum.Enum):
    
    QTQ = b'\x01' # QueueTxBufferRequest
    QTR = b'\x81' # QueueTxBufferResponse
    RRQ = b'\x02' # RetrieveRxBufferRequest
    RRR = b'\x82' # RetrieveRxBufferResponse
    GSQ = b'\x03' # GetStatusRequest
    GSR = b'\x83' # GetStatusResponse
    PTQ = b'\x04' # PushTxBufferRequest
    PTR = b'\x84' # PushTxBufferResponse
    PRQ = b'\x05' # PopRxBufferRequest
    PRR = b'\x85' # PopRxBufferResponse
    SSQ = b'\x06' # SendStatusRequest
    SSR = b'\x86' # SendStatusResponse

    @staticmethod
    def get_response(packet_type):
        # Get the response packet type for the request packet type.
        request_response_map = {
            PT.QTQ: PT.QTR,
            PT.QTR: PT.QTR,
            PT.RRQ: PT.RRR,
            PT.RRR: PT.RRR,
            PT.GSQ: PT.GSR,
            PT.GSR: PT.GSR,
            PT.PTQ: PT.PTR,
            PT.PTR: PT.PTR,
            PT.PRQ: PT.PRR,
            PT.PRR: PT.PRR,
            PT.SSQ: PT.SSR,
            PT.SSR: PT.SSR
        }

        if packet_type not in request_response_map:
            raise ValueError(f"Invalid packet type: {packet_type}")

        return request_response_map.get(packet_type)
    
    @staticmethod
    def get_type_name(packet_type: bytes) -> str:
        type_name_map = {
            PT.QTQ: "QueueTxBufferRequest",
            PT.QTR: "QueueTxBufferResponse",
            PT.RRQ: "RetrieveRxBufferRequest",
            PT.RRR: "RetrieveRxBufferResponse",
            PT.GSQ: "GetStatusRequest",
            PT.GSR: "GetStatusResponse",
            PT.PTQ: "PushTxBufferRequest",
            PT.PTR: "PushTxBufferResponse",
            PT.PRQ: "PopRxBufferRequest",
            PT.PRR: "PopRxBufferResponse",
            PT.SSQ: "SendStatusRequest",
            PT.SSR: "SendStatusResponse"
        }

        if packet_type not in type_name_map:
            raise ValueError(f"Invalid packet type: {packet_type}")

        return type_name_map.get(packet_type)


# Packet field.
# Value is the name of the field in the packet.
class PF(enum.Enum):
    PREAMBLE = ''
    TYPE = 'packet_type'
    SEQ = 'seq'
    SIZE = 'packet_size_raw'
    IS_SUCCESS = 'is_success'
    STATUS = 'status'
    PAYLOAD = 'payload'

# Packet content
class PC(enum.Enum):
    PREAMBLE = b'\x55\x55\x55\x55\x55\x55\x55\xD5' # Preamble
    SUCCESS = b'\x02' # Success
    FAILURE = b'\x01' # Failure
    EMPTY = b'\x00' # Empty

# Packet field format without preamble
@dataclass
class Packet:
    packet_type: bytes = b''
    seq: bytes = b''
    packet_size_raw: bytes = b'' # Needs to be set to the size of the all optional fields.
    is_success: Optional[bytes] = b''
    status: Optional[bytes] = b''
    payload: Optional[bytes] = b''

    fields_in_order = [PF.TYPE, PF.SEQ, PF.SIZE, PF.IS_SUCCESS, PF.STATUS, PF.PAYLOAD]

    # Size of each field in the packet.
    field_size = {
        PF.TYPE: 1,
        PF.SEQ: 1,
        PF.SIZE: 1,
        PF.IS_SUCCESS: 1,
        PF.STATUS: 1,
        PF.PAYLOAD: 0, # must be set to 0 for parse_bytes_to_packet
    }

    # Necessary fields for the packet. Order is important for parsing.
    necessary_fields = [PF.TYPE, PF.SEQ, PF.SIZE]

    # For_read_packet_and_parse function, fields must be in the order of the packet.
    optional_fields = {
        PT.QTR: [PF.IS_SUCCESS],
        PT.RRR: [PF.PAYLOAD],
        PT.GSR: [PF.IS_SUCCESS, PF.STATUS],
        PT.SSQ: [PF.STATUS],
        PT.PRQ: [PF.PAYLOAD],
    }

    def get(self, field: PF) -> bytes:
        return getattr(self, field.value, b'')
    
    def set(self, field: PF, value: bytes) -> None:
        setattr(self, field.value, value)

    def __bytes__(self):
        data = b''
        for b in map(lambda x: self.get(x), Packet.fields_in_order):
            if not b:
                continue
            if type(b) == int:
                data += bytes([b])
            else:
                data += b
        return data

    def is_valid(self) -> bool:
        for f in Packet.necessary_fields:
            if not self.get(f):
                return False
        
        if self._get_optional_packet_size_from_fields() != int.from_bytes(self.packet_size_raw):
            return False

        return True
            
    def complete_packet_size(self) -> None:
        self.packet_size_raw = self._get_optional_packet_size_from_fields().to_bytes(1, byteorder='big')

    @staticmethod
    def parse_bytes_to_packet(data: bytes, fields: list, field_size: int, packet = None):
        if len(data) != field_size:
            raise ValueError("Not enough data to parse packet")

        if packet == None:
            packet = Packet()

        offset = 0

        for f in fields:
            packet.set(f, data[offset : offset + Packet.field_size[f]])
            offset += Packet.field_size[f]

        if PF.PAYLOAD in fields:
            packet.payload = data[offset:]

        return packet
    
    @staticmethod
    def get_necessary_packet_size() -> int:
        return sum([Packet.field_size[f] for f in Packet.necessary_fields])
    

    def _get_optional_packet_size_from_fields(self) -> int:
        size = 0
        for f in Packet.fields_in_order:
            if not f in self.necessary_fields:
                size += len(self.get(f)) if self.get(f) != None else 0
        return size
    

class Badge:
    # Badge status
    class BS(enum.Enum):
        EMPTY_RX = 0
        EMPTY_TX = 1
        FULL_RX = 2
        FULL_TX = 3

    status_reject_packet_type_map = {
        # status set -> packet_type set

        frozenset({BS.EMPTY_RX}): {PT.RRQ},
        frozenset({BS.FULL_TX}): {PT.QTQ},
    }

    status_transition_map = { 
        # packet_type -> list[(condition(PF, value) list, pop status set, push status set), ..]
        # match only once in order.

        PT.PTQ: [([], {BS.FULL_TX}, set())],                                    # PushTxBufferRequest
        PT.PRQ: [([], {BS.EMPTY_RX}, set())],                                   # PopRxBufferRequest
        PT.QTR: [([(PF.IS_SUCCESS, PC.FAILURE.value)], set(), {BS.FULL_TX})],   # QueueTxBufferResponse
        PT.RRR: [([(PF.SIZE, PC.EMPTY.value)], set(), {BS.EMPTY_RX})],          # RetrieveRxBufferResponse
    }

    status = set()

    def check_packet_type_accept(self, packet_type: PT) -> bool:
        for s in self.status_reject_packet_type_map.keys():
            if s <= self.status and packet_type in self.status_reject_packet_type_map[s]:
                return False

        return True
    
    def update_status(self, packet: Packet) -> None:        
        for condition, pop_status, push_status in self.status_transition_map.get(PT(packet.get(PF.TYPE)), []):
            if all(packet.get(f) == v for f, v in condition):
                self.status -= pop_status
                self.status |= push_status
                return # match only once
            
class WriteDataIncompleteError(Exception):
    def __init__(self, expect, result):
        self.expect_len = expect
        self.result_len = result
        
    def __str__(self):
        return f"Writing data incomplete: expect writing data size={self.expect_len}, but result data size={self.result_len}"
    
class PacketQue:
    def __init__(self, maxsize: int = 1000, stay_timeout = 0.3):
        self.que = list()
        self.maxsize = maxsize
        self.stay_timeout = stay_timeout
        self.lock = asyncio.Lock()

    async def put(self, packet: bytes):
        async with self.lock:
            current_time = time.time()

            while len(self.que) != 0:
                if self.que[0][0] == packet:
                    self.que.pop(0)
                    return False
                elif (current_time - self.que[0][1]) < self.stay_timeout:
                    break

                self.que.pop(0)  # Remove the oldest packet

            for i in range(1, len(self.que)):
                if self.que[i][0] == packet:
                    self.que.pop(i)
                    return False
                
            if len(self.que) >= self.maxsize:
                logger.debug(f"Packet queue is full, dropping oldest packet: {self.que[0][0]}")
                self.que.pop(0)
                
            self.que.append((packet, current_time))
            return True
        

class IrInterface:
    def __init__(self, config: Config):
        self.port = config.get(key="usb_port")
        self.baudrate = config.get(key="usb_baudrate")
        self.usb_timeout = config.get(key="usb_timeout", default=1)
        self.wait_timeout = config.get(key="wait_timeout", default=3)
        self.failure_try = config.get(key="usb_failure_try", default=3)
        self.failure_wait = config.get(key="usb_failure_wait", default=0.1)
        self.packet_que_max = config.get(key="packet_que_max", default=1000)
        self.duplex = config.get(key="duplex", default=True)
        self.seq_set = set()
        self.current_seq = 0
        self.waiting_que_table = {} # (PT, seq) -> asyncio.Queue
        self.write_lock = asyncio.Lock()
        self.read_lock = asyncio.Lock()
        self.half_duplex_lock = asyncio.Lock()
        self.badge_status: Packet = None
        self.badge = Badge()
        self.read_packet_que = asyncio.Queue(maxsize=self.packet_que_max)
        self.serial: serial.Serial = None
        self.send_payload_buffer = dict() # payload -> send time
        self.recv_packet_que = PacketQue(maxsize=self.packet_que_max, stay_timeout=config.get(key="packet_stay_timeout", default=0.3))

    async def trigger_send_packet(self, data: bytes, packet_type = PT.QTQ, wait_response = True) -> bool | bytes:
        # Triggers the ir interface to send a packet.
        # Returns True if sent successfully, False otherwise.
        valid_packet_types = [PT.QTQ, PT.RRQ, PT.GSQ]
        if packet_type not in valid_packet_types:
            raise ValueError(f"Invalid packet type: {packet_type}. Must be one of {valid_packet_types}")

        for _ in range(self.failure_try):          
            try:

                result: bool | Packet = await self._write_packet(data, packet_type, wait_response=wait_response)

                if packet_type == PT.QTQ and (result == True or (type(result) == Packet and result.is_success == PC.SUCCESS.value)):
                    # If QueueTxBufferRequest is sent successfully, update the send_payload_buffer
                    self.send_payload_buffer[data] = time.time()

                if type(result) == Packet: # wait response case

                    if result.packet_size_raw == PC.EMPTY.value: # RetrieveRxBufferRequest case
                        return False
                    
                    if result.payload and type(result.payload) == bytes: # RetrieveRxBufferRequest case
                        return result.payload
                    
                    if result.is_success == PC.SUCCESS.value and result.status and type(result.status) == bytes: # GetStatusRequest case
                        return result.status
                    
                    return result.is_success == PC.SUCCESS.value # QueueTxBufferRequest case
                
                elif result == True: # no wait response case
                    return True
                
                # else re-send

            except serial.serialutil.SerialException: # raise serial connection broken error
                raise
            except Exception as e:
                print(f"Error sending packet: {type(e)}, {e}")
            
            await asyncio.sleep(self.failure_wait)

        return False
    

    async def get_next_packet(self) -> bytes:
        # Wait until the next packet arrives, then return its raw data.    
        for _ in range(self.failure_try):
            try:     
                while True:
                    await self._read_until_response(self.read_packet_que)
                    data = self.read_packet_que.get_nowait().payload
                    if await self.recv_packet_que.put(data):
                        return data

            except serial.serialutil.SerialException: # raise serial connection broken error
                raise
            except Exception as e:
                print(f"Error getting packet: {type(e)}, {e}")

            await asyncio.sleep(self.failure_wait)

        return b''
    

    async def _write_packet(self, data: bytes, packet_type = PT.QTQ, wait_response = True, lock = True) -> bool | Packet:
        # will return reponse if exists
        async def wait_func():
            if not self.badge.check_packet_type_accept(packet_type):
                return False

            packet = Packet(packet_type=packet_type.value, seq=self._get_seq() if wait_response else b'\x00', payload=data)
            packet.complete_packet_size() 

            logger.debug(f"Sending packet: \ntype = {PT.get_type_name(packet_type)} \npayload = {data} \npacket = {packet.__bytes__()}")

            if wait_response:
                return await self._write_packet_and_wait_for_response(packet)
            else:
                await self._write(packet.__bytes__(), lock=lock)
                return True
        
        return await asyncio.wait_for(wait_func(), timeout=self.wait_timeout)
    
    async def _write_packet_and_wait_for_response(self, packet: Packet) -> Packet:
        waiting_que_key = self._get_waiting_que_key(packet)

        if not waiting_que_key in self.waiting_que_table:
            self.waiting_que_table[waiting_que_key] = asyncio.Queue(maxsize=1)
        else:
            raise ValueError("Waiting queue already exists")
        
        await self._write(packet.__bytes__(), lock=True)

        try:
            logger.debug(f"Waiting for response: packet_type = {PT.get_type_name(waiting_que_key[0])}, seq = {waiting_que_key[1]}")
            await self._read_until_response(self.waiting_que_table[waiting_que_key])
            return self.waiting_que_table[waiting_que_key].get_nowait()
        finally:
            self._remove_seq(packet.seq)
            self.waiting_que_table.pop(waiting_que_key, None)

    async def _read_until_response(self, waiting_que: asyncio.Queue) -> None:
        # Must ensure waiting_que is exists
        async def wait_func():
            while waiting_que.empty():
                packet = await self._read_packet()
                self.badge.update_status(packet)
                
                match PT(packet.packet_type):
                    case PT.QTR | PT.RRR | PT.GSR: # QueueTxBufferResponse | RetrieveRxBufferResponse | GetStatusResponse
                        waiting_que_key = self._get_waiting_que_key(packet)

                        if waiting_que_key in self.waiting_que_table and not self.waiting_que_table[waiting_que_key].full():
                            self.waiting_que_table[waiting_que_key].put_nowait(packet)

                    case PT.PTQ | PT.SSQ: # PushTxBufferRequest | SendStatusRequest
                        self._response(packet)

                    case PT.PRQ:        # PopRxBufferRequest
                        self._response(packet)
                        self.read_packet_que.put_nowait(packet)

                await asyncio.sleep(0) # Yield control to allow other tasks to run
        
        await asyncio.wait_for(wait_func(), timeout=self.wait_timeout)

    async def _read_packet(self) -> Packet:
        async with self._get_lock(read=True):
            await self._read_until_preamble()
            data = await self._read(Packet.get_necessary_packet_size())
            packet = Packet.parse_bytes_to_packet(data, Packet.necessary_fields, Packet.get_necessary_packet_size())
            logger.debug(f"Received necessary packet fields: \ntype = {PT.get_type_name(PT(packet.packet_type))} \nseq = {packet.seq} \nsize = {packet.packet_size_raw}")
            data = await self._read(int.from_bytes(packet.packet_size_raw))
            packet = Packet.parse_bytes_to_packet(data, Packet.optional_fields[PT(packet.packet_type)], int.from_bytes(packet.packet_size_raw), packet=packet)
            
        if not packet.is_valid():
            raise ValueError("Received invalid packet")

        logger.debug(f"Received packet: \ntype = {PT.get_type_name(PT(packet.packet_type))} \npayload = {packet.payload} \npacket = {packet.__bytes__()}")
        return packet

    def _response(self, packet: Packet) -> None:
        # Only for PTQ, PRQ, SSQ
        r_packet_type_map = {
            PT.PTQ: PT.PTR,
            PT.PRQ: PT.PRR,
            PT.SSQ: PT.SSR
        }
        r_packet = Packet()
        r_packet.packet_type = r_packet_type_map[PT(packet.packet_type)].value
        r_packet.seq = packet.seq
        
        match PT(packet.packet_type):
            case PT.PRQ: # PopRxBufferRequest
                r_packet.is_success = PC.SUCCESS.value
        
        r_packet.complete_packet_size()

        asyncio.create_task(self._write(r_packet.__bytes__(), lock=True))

    def _get_waiting_que_key(self, packet: Packet) -> tuple:
        # Get the waiting queue key for the packet.
        # Convert packet_type to response packet type for request.        
        return (PT.get_response(PT(packet.packet_type)), packet.seq)

    def _get_lock(self, read: bool) -> asyncio.Lock:
        if read:
            return self.read_lock if self.duplex else self.half_duplex_lock
        return self.write_lock if self.duplex else self.half_duplex_lock


    def _get_seq(self) -> bytes:
        seq_limit = 2 ** (8 * Packet.field_size[PF.SEQ])
        for _ in range(seq_limit):
            self.current_seq = (self.current_seq + 1) % seq_limit
            if self.current_seq.to_bytes(1, byteorder='big') not in self.seq_set:
                break
        else:
            raise RuntimeError("No available sequence numbers")

        self.seq_set.add(self.current_seq.to_bytes(1, byteorder='big'))
        return self.current_seq.to_bytes(1, byteorder='big')


    def _remove_seq(self, seq: bytes):
        if seq in self.seq_set:
            self.seq_set.remove(seq)


    async def _read_until_preamble(self, timeout = None, lock = False) -> bool:
        plen = len(PC.PREAMBLE.value)
        
        if plen <= 0:
            return True
        
        async def loop() -> bool:
            timer = time.time()
            byte = b''
            while True:
                byte += await self._read(plen - len(byte))
                if byte == PC.PREAMBLE.value:
                    logger.debug("Successfully read preamble")
                    return True

                while PC.PREAMBLE.value[0] in byte:
                    i = byte.find(PC.PREAMBLE.value[0])

                    if not byte[i:] in PC.PREAMBLE.value:
                        byte = byte[i + 1:] 
                    else:
                        byte = byte[i:]
                        break
                else:
                    byte = b''
                
                if timeout != None and time.time() - timer >= timeout:
                    return False

        if lock:
            async with self._get_lock(read=True):
                return await loop()
            
        return await loop()


    async def _write(self, data: bytes, lock = False) -> None:
        if len(data) <= 0:
            return

        written_len = 0

        if lock:
            async with self._get_lock(read=False):
                written_len = await asyncio.wait_for(asyncio.to_thread(self.serial.write, PC.PREAMBLE.value + data), timeout=None)
        else:
            written_len = await asyncio.wait_for(asyncio.to_thread(self.serial.write, PC.PREAMBLE.value + data), timeout=None)

        if written_len != len(PC.PREAMBLE.value + data):
            raise WriteDataIncompleteError(len(PC.PREAMBLE.value + data), written_len)
        else:
            logger.debug(f"Successfully written data {PC.PREAMBLE.value + data}")
        

    async def _read(self, size: int, lock = False) -> bytes:
        if size <= 0:
            return b''
        data = b''
        
        if lock:
            async with self._get_lock(read=True):
                data = await asyncio.wait_for(asyncio.to_thread(self.serial.read, size), timeout=None)
        else:
            data = await asyncio.wait_for(asyncio.to_thread(self.serial.read, size), timeout=None)

        if len(data) == 0:
            logger.debug(f"No data Received")
        else:
            logger.debug(f"Successfully read data {data}")

        return data

    async def __aenter__(self):
        try:
            self.serial = await asyncio.wait_for(asyncio.to_thread(serial.Serial,
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.usb_timeout,
                write_timeout=self.usb_timeout
            ), timeout=self.usb_timeout)
            return self
        except serial.SerialException as e:
            raise RuntimeError(f"Failed to initialize serial port: {e}")
        except asyncio.TimeoutError as e:
            raise RuntimeError("Timeout while initializing serial port: {e}")
            

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.serial is not None:
            try:
                await asyncio.to_thread(self.serial.close)
                await asyncio.sleep(0)
            except Exception as e:
                logger.error(f"Error closing serial port: {e}")
            finally:
                self.serial = None


async def test():
    async with IrInterface(config=config) as ir:
        print("Test QTQ response:", await ir.trigger_send_packet(b'123'))
        print("Listening:")
        while 1:
            result = await ir.get_next_packet()
            if result:
                print(result)

            if result == b'\x00\x05H\x02\x8b\x0f\x7f]\x8f\x00\\\xfc\xca$\xbb\xe98\xae\x02\x128\xa2\xf5H':
                await ir.trigger_send_packet(b'\x00\x03\xac`7Nc\xfe')
            await asyncio.sleep(1)



if __name__ == '__main__':
    config = Config("config.yaml")
    asyncio.run(test())
    
        
    



    
        

