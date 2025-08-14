

"""
USB Serial Commands:

Necessary Fields:
Preamble - 8 bytes. 0xD5 0x55 0x55 0x55 0x55 0x55 0x55 0x55

PacketType - 1 byte
    base station to badge (to ir):
        * 0x01 for QueueTxBufferRequest
        * 0x81 for QueueTxBufferResponse
        * 0x02 for RetrieveRxBufferRequest 
        * 0x82 for RetrieveRxBufferResponse
        * 0x03 for GetStatusRequest
        * 0x83 for GetStatusResponse
    
    base station to badge (to cross board, use trigger_send_packet(..., to_cross_board=True)):
        * 0x11 for QueueTxBufferRequest
        * 0x91 for QueueTxBufferResponse
        ... (other cross board commands)

    base station to badge (else, print on badge, use trigger_send_packet(..., print_on_badge=True)):
        * 0x07 for PrintOnBadgeRequest (No response)

    badge to base station (from ir):
        * 0x04 for PushTxBufferRequest
        * 0x84 for PushTxBufferResponse
        * 0x05 for PopRxBufferRequest
        * 0x85 for PopRxBufferResponse
        * 0x06 for SendStatusRequest
        * 0x86 for SendStatusResponse

    badge to base station (from cross board):
        * 0x14 for PushTxBufferRequest
        * 0x94 for PushTxBufferResponse
        ... (other cross board commands)

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
from typing import Optional, Dict
import logging
from abc import abstractmethod

FORMAT = '%(levelname)s %(module)s %(lineno)d:%(message)s'

logging.basicConfig(level=logging.INFO, format=FORMAT)
logger = logging.getLogger(__name__)

# Information prefixes in PacketType.
class PT_INFO(enum.Enum):
    CROSS_BOARD = 16 # Cross board command prefix = 0x10
    # Add more information prefixes if needed in the future.
    
    @staticmethod
    def add_infos(packet_type: bytes, infos: list) -> bytes:
        packet_type = int.from_bytes(packet_type, byteorder='big')

        for info in infos:
            packet_type = packet_type | info.value

        return packet_type.to_bytes(1, byteorder='big')
    

    # return bools indicating whether the info is in the packet_type.
    @staticmethod
    def is_infos_in(packet_type: bytes, infos) -> bool | list[bool]:
        packet_type = int.from_bytes(packet_type, byteorder='big')

        if type(infos) == PT_INFO:
            return (packet_type & infos.value) != 0
        
        return [((packet_type & info.value) != 0) for info in infos]

    # return list of PT_INFO, each PT_INFO indicates whether the info is in the packet_type.
    @staticmethod
    def get_all_info(packet_type: bytes) -> list:
        b_array = PT_INFO.is_infos_in(packet_type, list(PT_INFO))
        return [info for info, present in zip(list(PT_INFO), b_array) if present]

    @staticmethod
    def remove_infos(packet_type: bytes, infos: list) -> bytes:
        packet_type = int.from_bytes(packet_type, byteorder='big')

        for info in infos:
            packet_type = packet_type & ~(info.value)

        return packet_type.to_bytes(1, byteorder='big')

    
    @staticmethod
    def remove_all_info(packet_type: bytes) -> bytes:
        return PT_INFO.remove_infos(packet_type, list(PT_INFO))
    
    # return int representing sum of all info value in the input bytes.
    @staticmethod
    def get_info_value(packet_type: bytes) -> int:
        ori_pt = int.from_bytes(PT_INFO.remove_all_info(packet_type), byteorder='big')
        return ori_pt ^ int.from_bytes(packet_type, byteorder='big')

    @staticmethod
    def info_transfer(from_pt: bytes, to_pt: bytes) -> bytes:
        result = PT_INFO.get_info_value(from_pt) | int.from_bytes(to_pt, byteorder='big')
        return result.to_bytes(1, byteorder='big')
    

# PacketType without prefix.
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
    PBR = b'\x07' # PrintOnBadgeRequest

    def get_response(self):
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
            PT.SSR: PT.SSR,
            PT.PBR: PT.PBR
        }

        return request_response_map.get(self)

    def get_type_name(self) -> str:
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
            PT.SSR: "SendStatusResponse",
            PT.PBR: "PrintOnBadgeRequest"
        }

        return type_name_map.get(self)
    
# PacketType with prefix.
class PTP(bytes):
    def get_PT(self) -> PT:
        # Get the packet type without the prefix.
        return PT(PT_INFO.remove_all_info(self))
    
    def get_all_info(self) -> list:
        return PT_INFO.get_all_info(self)
    
    @staticmethod
    def from_PT_and_infos(packet_type: PT, infos = []): 
        return PTP(PT_INFO.add_infos(packet_type.value, infos))
        

    def get_infos_from_other(self, packet_type: bytes):
        return PTP(PT_INFO.info_transfer(packet_type, self))

    def transfer_infos_to_other(self, packet_type: bytes):
        return PTP(PT_INFO.info_transfer(self, packet_type))
    
    def get_type_name(self):
        result = self.get_PT().get_type_name() + " ("
        result += ", ".join([info.name for info in self.get_all_info()])
        result += ")"
        return result


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
    packet_type: PTP = b''
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
        if field == PF.TYPE:
            self.packet_type = PTP(value)

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
            packet.set(f, data[offset : offset + Packet.field_size.get(f)])
            offset += Packet.field_size.get(f)

        if PF.PAYLOAD in fields:
            packet.payload = data[offset:]

        return packet
    
    @staticmethod
    def get_necessary_packet_size() -> int:
        return sum([Packet.field_size.get(f) for f in Packet.necessary_fields])
    

    def _get_optional_packet_size_from_fields(self) -> int:
        size = 0
        for f in Packet.fields_in_order:
            if not f in self.necessary_fields:
                size += len(self.get(f)) if self.get(f) != None else 0
        return size

class BackgroundRunner:
    raise_exception = False  # If True, will raise exception in run_forever() if any error occurs. Else will log the error and restart the task.
    restart_time = 0.5                 

    def  __init__(self):
        self.run_task: asyncio.Task = None
        self.is_running = False

    def run(self):
        # Run the background task.
        if self.run_task is not None:
            raise RuntimeError(f"{self.__class__.__name__} is already running")
        
        self.is_running = True
        self.run_task = asyncio.create_task(self.run_forever())

    async def run_forever(self):
        self.is_running = True
        logger.info(f"{self.__class__.__name__} start")
        while self.is_running:
            try:
                await self._run_forever()
            except Exception as e:
                logger.error(f"Error in {self.__class__.__name__}: {e}")
                if self.raise_exception:
                    raise
                else:
                    logger.warning(f"Restarting {self.__class__.__name__} after {self.restart_time} seconds")
                    await asyncio.sleep(self.restart_time)
                    self.restart_init()
                    logger.info(f"{self.__class__.__name__} start")


        logger.info(f"{self.__class__.__name__} is stopped")


    # Override this method to implement the background task logic.
    @abstractmethod
    async def _run_forever(self):
        pass

    async def stop(self):
        # Stop the background task.
        self._stop()
        try:
            if self.run_task is not None:
                await self.run_task
            self.run_task = None
        except Exception as e:
            logger.error(f"Error while stopping {self.__class__.__name__}: {e}")

    def _stop(self):
        self.is_running = False

    def restart_init(self):
        pass

# Handling serial reading.
class Reader(BackgroundRunner):
    raise_exception = True # raise error to IOHandler and restart it there

    def __init__(self, serial: serial.Serial, pkg_que: asyncio.Queue[Packet]):
        super().__init__()
        self.serial = serial
        self.pkg_que = pkg_que

    async def _run_forever(self):
        packet = await self._read_packet()

        if packet is None:
            return self._stop()
        elif packet is False:
            return
        
        self.pkg_que.put_nowait(packet)

    async def _read_packet(self) -> Packet | None | bool:
        if not await self._read_until_preamble():
            return None

        data = await self._read(Packet.get_necessary_packet_size())
        packet = Packet.parse_bytes_to_packet(data, Packet.necessary_fields, Packet.get_necessary_packet_size())
        data = await self._read(int.from_bytes(packet.packet_size_raw))
        packet_type = packet.packet_type.get_PT()
        packet = Packet.parse_bytes_to_packet(data, Packet.optional_fields.get(packet_type, []), int.from_bytes(packet.packet_size_raw), packet=packet)
            
        if not packet.is_valid():
            logger.warning("Received invalid packet, dropping it")
            return False

        logger.debug(f"Received packet: \ntype = {packet.packet_type.get_type_name()} \npayload = {packet.payload} \npacket = {packet.__bytes__()}")
        return packet

    async def _read_until_preamble(self) -> bool:
        plen = len(PC.PREAMBLE.value)
        
        if plen <= 0:
            return True
        
        byte = b''
        while self.is_running:
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
        return False

    async def _read(self, size: int) -> bytes:
        if size <= 0:
            return b''
        
        data = await asyncio.to_thread(self.serial.read, size)

        if len(data) != 0:
            logger.debug(f"Successfully read data {data}")

        return data

# Handling serial writing.
class Writer(BackgroundRunner):
    raise_exception = True # raise error to IOHandler and restart it there

    def __init__(self, serial: serial.Serial, pkg_que: asyncio.Queue[Packet]):
        super().__init__()
        self.serial = serial
        self.pkg_que: asyncio.Queue[Packet] = pkg_que

    async def _run_forever(self):
        packet: Packet = await self.pkg_que.get()

        if packet is None:
            return self._stop()

        logger.debug(f"Sending packet: \ntype = {packet.packet_type.get_type_name()} \npayload = {packet.payload} \npacket = {packet.__bytes__()}")

        written_bytes = PC.PREAMBLE.value + packet.__bytes__()
        written_len = await asyncio.to_thread(self.serial.write, written_bytes)

        while written_len != len(written_bytes):
            logger.warning(f"Writing data incomplete: expect writing data size={len(written_bytes)}, but result data size={written_len}. Try to write again.")
            written_len += await asyncio.to_thread(self.serial.write, written_bytes[written_len:])
        else:
            logger.debug(f"Successfully written data {written_bytes}")

    async def stop(self):
        self.pkg_que.put_nowait(None)
        await super().stop()

    
class FilterPool:
    def __init__(self, maxsize: int = 1000, stay_timeout = 0.3):
        self.pool = list()
        self.maxsize = maxsize
        self.stay_timeout = stay_timeout

    def print_pool(self):
        logger.debug("Filter_Pool:")
        for i, element in enumerate(self.pool):
            logger.debug(f"{i}: timestamp = {element[1]} packet = {element[0].__bytes__()}\n")

    def put(self, packet: bytes) -> bool:
        try:
            for i, (pkt, _) in enumerate(self.pool):
                if pkt == packet:
                    self.pool.pop(i)
                    return False
            self.pool.append((packet, time.time()))
            return True
        finally:
            self._prune_pool()
            self.print_pool()

    def _prune_pool(self):
        # Remove packets that have been in the pool for too long.
        current_time = time.time()
        self.pool = [(pkt, ts) for pkt, ts in self.pool if current_time - ts < self.stay_timeout]

        # Limit the size of the pool.
        if len(self.pool) > self.maxsize:
            self.pool = self.pool[-self.maxsize:]


# Filtering packets from Reader.
class Filter(BackgroundRunner):
    raise_exception = False

    def __init__(self, in_que: asyncio.Queue[Packet], out_que: asyncio.Queue[Packet]):
        super().__init__()
        self.in_que: asyncio.Queue[Packet] = in_que
        self.out_que: asyncio.Queue[Packet] = out_que
        self.filter_pool = FilterPool()

    async def _run_forever(self):
        packet: Packet = await self.in_que.get()

        if packet is None:
            return self._stop()

        if self.filter_pool.put(packet.__bytes__()):
            self.out_que.put_nowait(packet)
        else:
            logger.info(f"Filtered out packet: {packet.__bytes__()}")

    async def stop(self):
        self.in_que.put_nowait(None)
        await super().stop()

    def restart_init(self):
        while not self.in_que.empty():
            self.in_que.get_nowait()
        while not self.out_que.empty():
            self.out_que.get_nowait()
        self.filter_pool = FilterPool()


# Create and keep serial.Serial Obj in the entire lifetime of Reader and Writer.
class IOHandler(BackgroundRunner):
    raise_exception = False # Handle serial errors in Reader and Writer, try restart always.

    def __init__(self, config: Config,  read_pkg_que: asyncio.Queue, write_pkg_que: asyncio.Queue):
        super().__init__()
        self.config = config
        self.serial: serial.Serial = None
        self.port = config.get(key="usb_port")
        self.baudrate = config.get(key="usb_baudrate")
        self.usb_timeout = config.get(key="usb_timeout", default=1)
        self.pkg_que_max = config.get(key="packet_que_max", default=1000)
        self.read_pkg_que: asyncio.Queue[Packet] = read_pkg_que
        self.write_pkg_que: asyncio.Queue[Packet] = write_pkg_que
        self.reader: Reader = None
        self.writer: Writer = None

    async def _run_forever(self):
        s: serial.Serial = await asyncio.wait_for(asyncio.to_thread(serial.Serial,
            port=self.port,
            baudrate=self.baudrate,
            timeout=self.usb_timeout,
            write_timeout=self.usb_timeout
        ), timeout=self.usb_timeout)

        with s as self.serial:
            self.reader = Reader(self.serial, self.read_pkg_que)
            self.writer = Writer(self.serial, self.write_pkg_que)
            await asyncio.gather(
                self.reader.run_forever(),
                self.writer.run_forever()
            )

    async def stop(self):
        if self.writer is not None:
            await self.writer.stop()
        if self.reader is not None:
            await self.reader.stop()
        self.writer = None
        self.reader = None
        await super().stop()

class SeqObj:
    def __init__(self):
        self.event: asyncio.Event = asyncio.Event()
        self.waiting_time = 0
        self.waiting_task: asyncio.Task = None
        self.waiting_result: Packet = None

    def set_waiting_task(self):
        if self.waiting_task is None:
            self.waiting_task = asyncio.create_task(self._waiting_func())
            self.waiting_time = time.time()

    async def _waiting_func(self):
        await self.event.wait()
        return self.waiting_result

# Keeping maintain of sequence numbers table.
class SeqHandler(BackgroundRunner):
    raise_exception = False

    def __init__(self, timeout = 3, clear_interval = 1):
        super().__init__()
        self.seq_table: Dict[bytes, SeqObj] = dict()
        self.seq_size = Packet.field_size.get(PF.SEQ, 1)
        self.max_seq = 2 ** (8 * self.seq_size)
        self.current_seq = 1 # reserve 0 as non response pkg seq
        self.timeout = timeout  # Timeout for waiting sequence number
        self.clear_interval = clear_interval  # Interval for clearing expired sequences

    async def _run_forever(self): # Clear expired sequences
        current_time = time.time()
        for seq, seq_obj in list(self.seq_table.items()):
            if seq_obj.waiting_task is not None and current_time - seq_obj.waiting_time >= self.timeout:
                await self.reply(seq) # Timeout, reply None
        await asyncio.sleep(self.clear_interval)

    async def reply(self, seq: int | bytes, result: Optional[Packet] = None):
        seq_obj = self.get_obj(seq)
        if seq_obj is None:
            return
        seq_obj.waiting_result = result
        seq_obj.event.set()
        try:
            await seq_obj.waiting_task
        except Exception as e:
            logger.error(f"Error in reply: {e}")
        finally:
            self.remove_seq(seq)

    def get_idle_seq(self) -> tuple[bytes, SeqObj]:
        seq: bytes = b''
        for _ in range(self.max_seq - 1):
            self.current_seq = (self.current_seq + 1) % self.max_seq
            if self.current_seq == 0:
                self.current_seq = 1

            seq = self.convert_seq(self.current_seq)
            if seq not in self.seq_table:
                self.seq_table[seq] = SeqObj()
                break
        else:
            raise RuntimeError("No available sequence numbers")
        return seq, self.seq_table[seq]

    def get_obj(self, seq: int | bytes) -> Optional[SeqObj]:
        return self.seq_table.get(self.convert_seq(seq), None)

    def convert_seq(self, seq: int | bytes) -> bytes:
        if isinstance(seq, int):
            return seq.to_bytes(self.seq_size, byteorder='big')
        elif isinstance(seq, bytes):
            return seq
        raise TypeError("seq must be int or bytes")

    def remove_seq(self, seq: int | bytes):
        self.seq_table.pop(self.convert_seq(seq), None)

    def restart_init(self):
        self.seq_table.clear()

# Packet Handler to handle packets from Reader and process them based on their type.
class PacketHandler(BackgroundRunner):
    raise_exception = False

    def __init__(self, seq_handler: SeqHandler, in_que: asyncio.Queue[Packet], out_que: asyncio.Queue[Packet], write_pkg_que: asyncio.Queue[Packet]):
        super().__init__()
        self.seq_handler = seq_handler
        self.in_que = in_que
        self.out_que = out_que
        self.write_pkg_que = write_pkg_que

    async def _run_forever(self):
        packet = await self.in_que.get()
        if packet is None:
            return self._stop()

        packet_type = packet.packet_type.get_PT()
        match packet_type:
            case PT.QTR | PT.RRR | PT.GSR: # QueueTxBufferResponse | RetrieveRxBufferResponse | GetStatusResponse
                await self.seq_handler.reply(packet.seq, packet)

            case PT.PTQ | PT.SSQ: # PushTxBufferRequest | SendStatusRequest
                self._response(packet)

            case PT.PRQ:        # PopRxBufferRequest
                self._response(packet)
                self.out_que.put_nowait(packet)


    def send_packet(self, packet: Packet):
        packet.seq = self.seq_handler.convert_seq(0)  # Use 0 for non-response packets
        self.write_pkg_que.put_nowait(packet)

    async def send_packet_and_wait(self, packet: Packet) -> Optional[Packet]:
        # Write packet and wait for response.
        seq, seq_obj = self.seq_handler.get_idle_seq()
        packet.seq = seq
        self.write_pkg_que.put_nowait(packet)
        seq_obj.set_waiting_task()
        return await seq_obj.waiting_task

    async def get_packet(self):
        return await self.out_que.get()

    def _response(self, packet: Packet) -> None:
        r_packet = Packet()
        r_packet.packet_type = packet.packet_type.transfer_infos_to_other(packet.packet_type.get_PT().get_response().value)
        r_packet.seq = packet.seq

        match PT(packet.packet_type.get_PT()):
            case PT.PRQ: # PopRxBufferRequest
                r_packet.is_success = PC.SUCCESS.value
        
        r_packet.complete_packet_size()

        self.write_pkg_que.put_nowait(r_packet)

    async def stop(self):
        self.in_que.put_nowait(None)
        await super().stop()

    def restart_init(self):
        while not self.in_que.empty():
            self.in_que.get_nowait()
        while not self.out_que.empty():
            self.out_que.get_nowait()
        while not self.write_pkg_que.empty():
            self.write_pkg_que.get_nowait()
        self.seq_handler.restart_init()


class IrInterface:
    def __init__(self, config: Config):
        self.config = config
        self.failure_try = config.get(key="failure_try", default=3)
        self.failure_wait = config.get(key="failure_wait", default=0.1)
        self.pkg_que_max = config.get(key="packet_que_max", default=1000)
        self.read_pkg_que = asyncio.Queue(maxsize=self.pkg_que_max)
        self.filtered_pkg_que = asyncio.Queue(maxsize=self.pkg_que_max)
        self.non_response_pkg_que = asyncio.Queue(maxsize=self.pkg_que_max)
        self.write_pkg_que = asyncio.Queue(maxsize=self.pkg_que_max)
        self.io_handler = IOHandler(config, self.read_pkg_que, self.write_pkg_que)
        self.filter = Filter(self.read_pkg_que, self.filtered_pkg_que)
        self.seq_handler = SeqHandler()
        self.pkg_handler = PacketHandler(self.seq_handler, self.filtered_pkg_que, self.non_response_pkg_que, self.write_pkg_que)

    # response: ((bool is_success | bytes payload or status), list of PT_INFO)
    async def trigger_send_packet(self, data: bytes, packet_type = PT.QTQ, wait_response = True, to_cross_board = False, print_on_badge = False):
        # Triggers the ir interface to send a packet.
        # Returns True if sent successfully, False otherwise.
        valid_packet_types = [PT.QTQ, PT.RRQ, PT.GSQ, PT.PBR]
        if packet_type not in valid_packet_types:
            raise ValueError(f"Invalid packet type: {packet_type}. Must be one of {valid_packet_types}")
        
        infos = []
        if to_cross_board:
            infos.append(PT_INFO.CROSS_BOARD)

        if print_on_badge:
            packet_type = PT.PBR

        if packet_type == PT.PBR: # Print on badge request
            infos.clear() # Clear all infos, since print on badge does not need any infos.
            wait_response = False # Print on badge does not need response.


        packet = Packet(packet_type=PTP.from_PT_and_infos(packet_type, infos), payload=data)
        packet.complete_packet_size()

        for _ in range(self.failure_try):          
            try:

                if not wait_response:
                    self.pkg_handler.send_packet(packet)
                    return True, []
                
                result: Optional[Packet] = await self.pkg_handler.send_packet_and_wait(packet)

                if result is None: # Timeout or error
                    logger.warning("No response received, retrying...")
                    await asyncio.sleep(self.failure_wait)
                    continue


                if result.packet_size_raw == PC.EMPTY.value: # RetrieveRxBufferRequest return empty case
                    return False, result.packet_type.get_all_info()

                if result.payload and type(result.payload) == bytes: # RetrieveRxBufferRequest case
                    return result.payload, result.packet_type.get_all_info()

                if result.is_success == PC.SUCCESS.value and result.status and type(result.status) == bytes: # GetStatusRequest case
                    return result.status, result.packet_type.get_all_info()

                return (result.is_success == PC.SUCCESS.value), result.packet_type.get_all_info() # QueueTxBufferRequest case
                

            except Exception as e:
                logger.error(f"Error sending packet: {e}")
            
            await asyncio.sleep(self.failure_wait)

        return False, []

    # response: (bytes payload, list of PT_INFO)
    async def get_next_packet(self) -> bytes:
        # Wait until the next packet arrives, then return its raw data.    
        for _ in range(self.failure_try):
            try:     
                packet = await self.pkg_handler.get_packet()
                if packet != None:
                    return packet.payload, packet.packet_type.get_all_info()

            except Exception as e:
                import traceback
                traceback.print_exc()
                logger.error(f"Error getting packet: {e}")

            await asyncio.sleep(self.failure_wait)

        return b'', [] # No packet received after retries
    

    async def show_graphic(self, display_data: bytes):
        """
        Shows graphic on the base station's display.
        display_data is 16 bytes, each byte is a vertical column in the display.
        The LSB is the lowest LED.
        The first byte is the left most column.
        """
        return await self.trigger_send_packet(display_data, packet_type=PT.PBR, wait_response=False, print_on_badge=True)

    async def get_any_exception(self):
        """
        Returns any exception that occurred in the background tasks.
        This is useful for debugging and error handling.
        """
        if self.io_handler.run_task is not None and self.io_handler.run_task.done():
            await self.io_handler.run_task
        
        if self.filter.run_task is not None and self.filter.run_task.done():
            await self.filter.run_task

        if self.seq_handler.run_task is not None and self.seq_handler.run_task.done():
            await self.seq_handler.run_task

        if self.pkg_handler.run_task is not None and self.pkg_handler.run_task.done():
            await self.pkg_handler.run_task

    async def __aenter__(self):
        self.io_handler.run()
        self.filter.run()
        self.seq_handler.run()
        self.pkg_handler.run()
        return self
            

    async def __aexit__(self, *args, **kwargs):
        await self.io_handler.stop()
        await self.filter.stop()
        await self.seq_handler.stop()
        await self.pkg_handler.stop()


async def test():
    print_msg = b'\x12' * 16
    async with IrInterface(config=config) as ir:
        print(f"\nTest Print On Badge: {print_msg}")
        await ir.trigger_send_packet(print_msg, print_on_badge=True)
        print("\nTest QTQ response:", await ir.trigger_send_packet(b'123', to_cross_board=True))
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
    
        
    



    
        

