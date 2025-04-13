from typing import Optional, Iterator
from bson import Binary
from pymongo.asynchronous.database import AsyncDatabase
from crypto_auth_layer import CryptoAuthLayer
from schemas import IrPacket, IrPacketRequestSchema, IrPacketObject, Station, PacketType, PACKET_HASH_LEN, IR_USERNAME_LEN
from config import Config
import uuid


class PacketProcessor:
    def __init__(self, config: Config, crypto_auth: CryptoAuthLayer, db: AsyncDatabase):
        self.crypto_auth = crypto_auth
        self.config = config
        self.db = db
        self.stations = db["stations"]
        self.packets = db["packets"]
        self.users = db["users"]


    # ===== Interface to HTTP =====
    async def on_receive_packet(self, ir_packet: IrPacketRequestSchema, station: Station) -> None:
        pass


    async def has_packet_for_tx(self, station: Station) -> Iterator[IrPacketRequestSchema]:
        pass


    # ===== Interface for CryptoAuthLayer =====
    async def send_packet_to_user(self, ir_packet: IrPacket, username: int) -> uuid.UUID:
        """
        Send a packet to a particular user. PacketProcessor will queue it for sending, and when activity from the given user is observed on a base station, packet will be directed to it.
        Will return immediately, and return the UUID of the packet.

        ir_packet's packet_id field can be left empty, if empty, will auto populate.
        """
        pass

    # ===== Internal methods =====
    def packet_hash(self, ir_packet: IrPacketRequestSchema) -> bytes:
        """
        Get the packet hash. TODO: This is a placeholder implementation and should be replaced with actual hashing logic.
        """
        return ir_packet.data[-(PACKET_HASH_LEN):]


    def get_packet_type(self, ir_packet: IrPacketRequestSchema) -> Optional[PacketType]:
        # Determine the type of packet based on its contents.
        # This is a placeholder implementation and should be replaced with actual logic.
        raw_type = ir_packet.data[0]
        packet_type = PacketType(raw_type)
        if packet_type in PacketType:
            return packet_type
        else:
            return None


    async def get_user_last_station_uuid(self, username: int) -> Optional[uuid.UUID]:
        # Get the station associated with a user.
        user = await self.users.find_one({"username": username})

        if user:
            return user.get("station_id")
        else:
            return None