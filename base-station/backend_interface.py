import uuid
from config import Config
from typing import Tuple


class BackendInterface:
    def __init__(self, config: Config):
        self.station_id = config.get(key="station_id")
        self.config = config

    async def send_received_packed(data: bytes, packet_id: uuid.UUID):
        # Call to send a packet received by IR interface to the backend.
        pass

    async def get_next_tx_packet() -> Tuple[bytes, uuid.UUID]:
        # Returns the next packet to transmit.
        # Blocks if none is available.
        pass

    # ===== Internal methods =====
    async def _tx_poll_task():
        # Runs continuously to get any new packets to transmit.
        pass
