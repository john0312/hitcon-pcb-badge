import uuid
import asyncio

class PacketProcessor:
    def __init__(self):
        self.backend = None
        self.ir = None
        # Avoid duplicate transmissions
        self.seen_packet_ids = set()
        self._rx_task = None
        self._tx_task = None

    async def _tx_stream_task(self):
        # Get packets from backend, check if already sent, if not send via IR
        while True:
            packets = await self.backend.get_next_tx_packet()
            for result in packets:
                packet_data, packet_id = result

                if packet_id in self.seen_packet_ids:
                    print(f"[TX] Ignoring duplicate packet {packet_id}")
                else:
                    print(f"[TX] Sending packet {packet_id} -> {packet_data}")
                    assert type(packet_data) == bytes, "Packet data must be bytes"
                    await self.ir.trigger_send_packet(packet_data)
                    self.seen_packet_ids.add(packet_id)
            if len(result) == 0:
                asyncio.sleep(1.0)

    async def _rx_stream_task(self):
        # Receive IR packets and upload to backend
        while True:
            packet_data = await self.ir.get_next_packet()
            packet_id = uuid.uuid4()
            print(f"[RX] Received IR packet -> {packet_data}, ID: {packet_id}")
            await self.backend.send_received_packet(packet_data, packet_id)

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
