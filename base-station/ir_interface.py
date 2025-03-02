class IrInterface:
    def __init__(self):
        pass

    async def trigger_send_packet(self, data: bytes) -> bool:
        # Triggers the ir interface to send a packet.
        # Returns True if sent successfully, False otherwise.
        pass

    async def get_next_packet(self) -> bytes:
        # Wait until the next packet arrives, then return its raw data.
        pass
