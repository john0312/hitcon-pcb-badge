import uuid
import asyncio
import ir_interface
import traceback
from config import Config
from packet_recorder import PacketRecorder

def station_key_to_id(station_key: str) -> str:
    station_id = '.'.join(station_key.split('.')[0:2])
    return station_id

class PacketProcessor:
    def __init__(self, config: Config):
        self.backend = None
        self.ir = None
        self.station_key_ir = config.get("station_key_ir")
        self.station_key_xb = config.get("station_key_xb")
        assert self.station_key_ir is not None
        assert self.station_key_xb is not None

        # Avoid duplicate transmissions
        self.seen_packet_ids = set()
        self._rx_producer_task = None
        self._rx_consumer_task = None
        self._tx_producer_task_ir = None
        self._tx_producer_task_xb = None
        self._tx_consumer_task = None
        self._rx_queue = asyncio.Queue(maxsize=100)
        self._tx_queue = asyncio.Queue(maxsize=100)
        self.recorder = PacketRecorder()

    async def _tx_producer_fn(self, station_key: str, is_cross_board: bool):
        """Get packets from backend and put them in TX queue"""
        station_id = station_key_to_id(station_key)
        while True:
            try:
                packets = await self.backend.get_next_tx_packet(station_key=station_key)
                duplicate_count = 0
                for packet_data, packet_id in packets:
                    if (packet_id, station_id) in self.seen_packet_ids:
                        print(f"[TX] Ignoring duplicate packet {packet_id}, {station_id}")
                        duplicate_count += 1
                    else:
                        print(f"[TX] Queuing packet {packet_id}, {station_id}")
                        await self.recorder.record_packet(packet_data, "TX", packet_id, station_id=station_id)
                        await self._tx_queue.put((packet_data, packet_id, is_cross_board))
                        self.seen_packet_ids.add((packet_id, station_id))
                if len(packets)-duplicate_count == 0:
                    await asyncio.sleep(2.0)
            except Exception as e:
                traceback.print_exc()
                print(f"Exception in PacketProcessor._tx_producer_fn({station_key}): {e}")
                raise

    async def _tx_consumer_fn(self):
        """Take packets from TX queue and send via IR"""
        while True:
            try:
                packet_data, packet_id, is_cross_board = await self._tx_queue.get()
                assert type(packet_data) == bytes, "Packet data must be bytes"
                await self.ir.trigger_send_packet(packet_data, to_cross_board=is_cross_board)
            except Exception as e:
                traceback.print_exc()
                print(f"Exception in PacketProcessor._tx_consumer_fn(): {e}")
                raise

    async def _rx_producer_fn(self):
        """Receive IR packets and put them in RX queue"""
        while True:
            try:
                packet_data, packet_infos = await self.ir.get_next_packet()
                if len(packet_data) == 0:
                    continue
                packet_infos = set(packet_infos)
                packet_id = uuid.uuid4()
                station_key = self.station_key_ir
                if ir_interface.PT_INFO.CROSS_BOARD in packet_infos:
                    station_key = self.station_key_xb
                station_id = station_key_to_id(station_key)
                print(f"[RX] Received IR packet -> {packet_data}, station_id: {station_id}, infos: {packet_infos}, ID: {packet_id}")
                await self.recorder.record_packet(packet_data, "RX", packet_id, station_id=station_id)
                await self._rx_queue.put((packet_data, packet_id, station_key))
            except Exception as e:
                traceback.print_exc()
                print(f"Exception in PacketProcessor._rx_producer_fn(): {e}")

    async def _rx_consumer_fn(self):
        """Take packets from RX queue and send to backend"""
        while True:
            try:
                packet_data, packet_id, station_key = await self._rx_queue.get()
                await self.backend.send_received_packet(packet_data, packet_id, station_key=station_key)
            except Exception as e:
                traceback.print_exc()
                print(f"Exception in PacketProcessor._rx_consumer_fn(): {e}")

    async def _disp_task_fn(self):
        while True:
            try:
                await asyncio.sleep(2.0)
                score = await self.backend.get_station_score(station_key=self.station_key_ir)
                if score is None:
                    # Restarting.
                    continue
                disp_data = self.map_score_to_disp_data(score)
                await self.ir.show_graphic(disp_data)
            except Exception as e:
                traceback.print_exc()
                print(f"Exception in _disp_task_fn(): {e}")

    def map_score_to_disp_data(self, score):
        score = score / 10.0
        score = score + 8
        score = max(0, score)
        score = min(16, score)
        res = []
        for i in range(16):
            v = 0
            if i < score:
                v = v | 0x06
            if i >= score:
                v = v | 0x60
            res.append(v)
        return bytes(res)

    async def __aenter__(self):
        # Start background tasks
        return self

    def start(self):
        self._tx_producer_task_ir = asyncio.create_task(self._tx_producer_fn(station_key=self.station_key_ir, is_cross_board=False))
        self._tx_producer_task_xb = asyncio.create_task(self._tx_producer_fn(station_key=self.station_key_xb, is_cross_board=True))
        self._tx_consumer_task = asyncio.create_task(self._tx_consumer_fn())
        self._rx_producer_task = asyncio.create_task(self._rx_producer_fn())
        self._rx_consumer_task = asyncio.create_task(self._rx_consumer_fn())
        self._disp_task = asyncio.create_task(self._disp_task_fn())

    async def __aexit__(self, exc_type, exc, tb):
        # Cancel background tasks
        tasks = [
            self._tx_producer_task_ir,
            self._tx_producer_task_xb,
            self._tx_consumer_task,
            self._rx_producer_task,
            self._rx_consumer_task
        ]
        for task in tasks:
            if task is not None:
                task.cancel()
                try:
                    await task
                except asyncio.CancelledError:
                    pass
