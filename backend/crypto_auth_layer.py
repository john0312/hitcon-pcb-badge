from typing import Optional
from schemas import IrPacket
from config import Config
from ecc_utils import ecc_sign, ecc_verify

class CryptoAuthLayer:
    def __init__(self, config: Config):
        self.config = config

    # ===== Generic methods for any other layers =====
    async def get_pubkey_by_username(username: int) -> Optional[int]:
        pass

    # ===== APIs for PacketProcessor =====
    async def on_packet_received(ir_packet: IrPacket):
        pass
