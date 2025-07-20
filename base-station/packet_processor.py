import uuid
import asyncio
import ir_interface
from packet_recorder import PacketRecorder

class PacketProcessor:
    def __init__(self):
        self.backend = None
        self.ir = None
        # Avoid duplicate transmissions
        self.seen_packet_ids = set()
        self._rx_producer_task = None
        self._rx_consumer_task = None
        self._tx_producer_task = None 
        self._tx_consumer_task = None
        self._rx_queue = asyncio.Queue(maxsize=100)
        self._tx_queue = asyncio.Queue(maxsize=100)
        self.recorder = PacketRecorder()

    async def _tx_producer_fn(self):
        """Get packets from backend and put them in TX queue"""
        while True:
            packets = await self.backend.get_next_tx_packet()
            duplicate_count = 0
            for packet_data, packet_id in packets:
                if packet_id in self.seen_packet_ids:
                    print(f"[TX] Ignoring duplicate packet {packet_id}")
                    duplicate_count += 1
                else:
                    print(f"[TX] Queuing packet {packet_id}")
                    await self.recorder.record_packet(packet_data, "TX", packet_id)
                    await self._tx_queue.put((packet_data, packet_id))
                    self.seen_packet_ids.add(packet_id)
            if len(packets)-duplicate_count == 0:
                await asyncio.sleep(2.0)

    async def _tx_consumer_fn(self):
        """Take packets from TX queue and send via IR"""
        while True:
            packet_data, packet_id = await self._tx_queue.get()
            assert type(packet_data) == bytes, "Packet data must be bytes"
            await self.ir.trigger_send_packet(packet_data)

    async def _rx_producer_fn(self):
        """Receive IR packets and put them in RX queue"""
        while True:
            packet_data = await self.ir.get_next_packet()
            packet_id = uuid.uuid4()
            print(f"[RX] Received IR packet -> {packet_data}, ID: {packet_id}")
            await self.recorder.record_packet(packet_data[0], "RX", packet_id)
            await self._rx_queue.put((packet_data[0], packet_id))

    async def _rx_consumer_fn(self):
        """Take packets from RX queue and send to backend"""
        while True:
            packet_data, packet_id = await self._rx_queue.get()
            await self.backend.send_received_packet(packet_data, packet_id)

    async def _disp_task_fn(self):
        try:
            while True:
                await asyncio.sleep(2.0)
                score = await self.backend.get_station_score()
                disp_data = self.map_score_to_disp_data(score)
                await self.ir.show_graphic(disp_data)
        except:
            import traceback
            print(f"Exception in _disp_task_fn")
            traceback.print_exc()    

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
        self._tx_producer_task = asyncio.create_task(self._tx_producer_fn())
        self._tx_consumer_task = asyncio.create_task(self._tx_consumer_fn())
        self._rx_producer_task = asyncio.create_task(self._rx_producer_fn())
        self._rx_consumer_task = asyncio.create_task(self._rx_consumer_fn())
        self._disp_task = asyncio.create_task(self._disp_task_fn())

    async def __aexit__(self, exc_type, exc, tb):
        # Cancel background tasks
        tasks = [
            self._tx_producer_task,
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
