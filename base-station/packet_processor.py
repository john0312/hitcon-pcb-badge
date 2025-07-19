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
        self._rx_task = None
        self._tx_task = None
        self.recorder = PacketRecorder()

    async def _tx_stream_task(self):
        # Get packets from backend, check if already sent, if not send via IR
        while True:
            packets = await self.backend.get_next_tx_packet()
            duplicate_count = 0
            for result in packets:
                packet_data, packet_id = result

                if packet_id in self.seen_packet_ids:
                    print(f"[TX] Ignoring duplicate packet {packet_id}")
                    duplicate_count += 1
                else:
                    print(f"[TX] Sending packet {packet_id} -> {packet_data}")
                    assert type(packet_data) == bytes, "Packet data must be bytes"
                    await self.ir.trigger_send_packet(packet_data)
                    self.seen_packet_ids.add(packet_id)
                    await self.recorder.record_packet(packet_data, "TX", packet_id)
            if len(packets)-duplicate_count == 0:
                await asyncio.sleep(2.0)

    async def _rx_stream_task(self):
        # Receive IR packets and upload to backend
        while True:
            packet_data = await self.ir.get_next_packet()
            packet_id = uuid.uuid4()
            print(f"[RX] Received IR packet -> {packet_data}, ID: {packet_id}")
            await self.backend.send_received_packet(packet_data[0], packet_id)
            await self.recorder.record_packet(packet_data[0], "RX", packet_id)

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
        self._tx_task = asyncio.create_task(self._tx_stream_task())
        self._rx_task = asyncio.create_task(self._rx_stream_task())

    async def __aexit__(self, exc_type, exc, tb):
        # Cancel background tasks
        if self._tx_task is not None:
            self._tx_task.cancel()
            try:
                await self._tx_task
            except asyncio.CancelledError:
                pass
        if self._rx_task is not None:
            self._rx_task.cancel()
            try:
                await self._rx_task
            except asyncio.CancelledError:
                pass
