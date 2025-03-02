from typing import Optional
from schemas import IrPacket


class CryptoAuthLayer:
    def __init__(self):
        pass

    # ===== Generic methods for any other layers =====
    async def get_pubkey_by_username(username: int) -> Optional[int]:
        pass

    # ===== APIs for PacketProcessor =====
    async def on_packet_received(ir_packet: IrPacket):
        pass
