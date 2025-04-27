from typing import Optional, AsyncIterator, Callable, Awaitable, Dict, ClassVar
from bson import Binary
from pymongo.asynchronous.database import AsyncDatabase
from crypto_auth import CryptoAuth
from schemas import IrPacket, IrPacketRequestSchema, IrPacketObject, Station, PacketType, PACKET_HASH_LEN, IR_USERNAME_LEN
from config import Config
import uuid
import time

class PacketProcessor:
    packet_handlers: ClassVar[Dict[PacketType, Callable[[IrPacket, Station], Awaitable[None]]]] = dict()

    def __init__(self, config: Config, crypto_auth: CryptoAuth, db: AsyncDatabase):
        self.crypto_auth = crypto_auth
        self.config = config
        self.db = db
        self.stations = db["stations"]
        self.packets = db["packets"]
        self.users = db["users"]


    @staticmethod
    def register_handler(type: PacketType, func: Callable[[IrPacket, Station], Awaitable[None]]) -> None:
        """
        Register a handler for a specific packet type.
        """
        PacketProcessor.packet_handlers[type] = func


    # ===== Interface to HTTP =====
    async def on_receive_packet(self, ir_packet: IrPacketRequestSchema, station: Station) -> None:
        if await self.handle_acknowledgment(ir_packet, station):
            # If the packet is an acknowledgment, we don't need to do anything else.
            return

        packet = IrPacket(
            packet_id=ir_packet.packet_id,
            data=bytes(ir_packet.data),
            station_id=station.station_id,
            to_stn=False
        )

        # verify the packet
        # it would throw an exception if the packet is invalid
        await self.crypto_auth.verify_packet(packet)

        if await self.handle_proximity(ir_packet):
            # If the packet is a proximity event, we don't need to do anything else.
            return

        hv = self.packet_hash(ir_packet)
        db_packet = IrPacketObject(packet_id=ir_packet.packet_id, data=Binary(ir_packet.data), hash=Binary(hv))

        # add the packet to the database
        result = await self.packets.insert_one(
            {"station_id": station.station_id},
            {"$push": {"rx": db_packet.model_dump()}}
        )

        # add packet ObjectId to station rx list
        await self.stations.update_one(
            {"station_id": station.station_id},
            {"$push": {"rx": result.inserted_id}}
        )


    async def has_packet_for_tx(self, station: Station) -> AsyncIterator[IrPacketRequestSchema]:
        packets = self.packets.find({"_id": {"$in": station.tx}})
        async for packet in packets:
            # Convert the packet to IrPacketRequestSchema and yield it.
            yield IrPacketRequestSchema(
                packet_id=packet["packet_id"],
                data=list(packet["data"])
            )


    # ===== Interface for GameLogic =====
    async def send_packet_to_user(self, ir_packet: IrPacket, username: int) -> uuid.UUID:
        """
        Send a packet to a particular user. PacketProcessor will queue it for sending, and when activity from the given user is observed on a base station, packet will be directed to it.
        Will return immediately, and return the UUID of the packet.

        ir_packet's packet_id field can be left empty, if empty, will auto populate.
        """

        packet_id = ir_packet.packet_id or uuid.uuid4()
        ir_packet.packet_id = packet_id
        ir_packet.to_stn = True
        ir_packet.station_id = await self.get_user_last_station_uuid(username)

        if ir_packet.station_id is None:
            # If the user is not associated with any station, we cannot send the packet?
            raise ValueError("User not associated with any station.")
        
        return await self.send_packet_to_station(ir_packet)


    async def send_packet_to_station(self, ir_packet: IrPacket) -> uuid.UUID:
        hv = self.packet_hash(ir_packet)
        
        db_packet = IrPacketObject(packet_id=ir_packet.packet_id, data=Binary(ir_packet.data), hash=Binary(hv), timestamp=int(time.time()))

        # add the packet to the database
        result = await self.packets.insert_one(
            db_packet.model_dump(exclude={"id"})
        )

        # add packet ObjectId to station tx list
        await self.stations.update_one(
            {"station_id": ir_packet.station_id},
            {"$push": {"tx": result.inserted_id}}
        )

        return ir_packet.packet_id


    # ===== Internal methods =====
    def packet_hash(self, ir_packet: IrPacketRequestSchema) -> bytes:
        """
        Get the packet hash. TODO: This is a placeholder implementation and should be replaced with actual hashing logic.
        """
        return bytes(ir_packet.data[-(PACKET_HASH_LEN):])


    async def handle_acknowledgment(self, ir_packet: IrPacketRequestSchema, station: Station) -> bool:
        # Handle acknowledgment from the base station.
        # This is where we would update the database or perform any other necessary actions.
        packet_type = self.get_packet_type(ir_packet)
        if packet_type == PacketType.kAcknowledge:
            hv = self.packet_hash(ir_packet)

            # Acknowledge the packet and remove it from the tx list.
            packets = await self.packets.find({"hash": hv}).to_list()
            result = await self.stations.update_one(
                {"station_id": station.station_id},
                {"$pull": {"tx": { "$in": list(map(lambda x: x["_id"], packets)) }}}
            )

            # Remove the packet from the database.
            await self.packets.delete_many({"hash": hv})
            
            return True

        return False


    async def handle_proximity(self, ir_packet: IrPacketRequestSchema, station: Station) -> bool:
        # Handle proximity packets.
        # This is where we would update the database or perform any other necessary actions.
        packet_type = self.get_packet_type(ir_packet)
        if packet_type == PacketType.kProximity:
            # Process proximity event
            offset = 1
            username = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)

            # update users' last station
            await self.users.update_one(
                {"username": username},
                {"$set": {"station_id": station.station_id}}
            )

            return True

        return False


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