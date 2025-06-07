import uuid
import asyncio

class PacketProcessor:
    def __init__(self, backend, ir):
        self.backend = backend      # BackendInterface
        self.ir = ir                # IRInterface
        self.seen_packet_ids = set()  # 避免重複發送

    async def _tx_stream_task(self):
        # Get packets from backend, check if already sent, if not send via IR
        while True:
            result = await self.backend.get_next_tx_packet()
            if result:
                packet_data, packet_id = result

                if packet_id in self.seen_packet_ids:
                    print(f"[TX] Ignoring duplicate packet {packet_id}")
                else:
                    print(f"[TX] Sending packet {packet_id} -> {packet_data}")
                    await self.ir.send(packet_data)
                    self.seen_packet_ids.add(packet_id)

            await asyncio.sleep(2)

    async def _rx_stream_task(self):
        # Receive IR packets and upload to backend
        while True:
            packet_data = await self.ir.receive()
            packet_id = uuid.uuid4()
            print(f"[RX] Received IR packet -> {packet_data}, ID: {packet_id}")
            await self.backend.send_received_packet(packet_data, packet_id)
