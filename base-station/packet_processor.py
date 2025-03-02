class PacketProcessor:
    def __init__(self, backend, ir):
        self.backend = backend
        self.ir = ir

    async def _tx_stream_task(self):
        # Continuously fetch a packet from backend interface, then check if it's a duplicate, if not, then send it to ir interface.
        # Also retry any failed previous tx.
        pass

    async def _rx_stream_task(self):
        # Receive any packet from ir interface then forward it to backend.
        pass
