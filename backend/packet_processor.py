from typing import Optional, AsyncIterator, Callable, Awaitable, Dict, ClassVar
from bson import Binary
from pymongo.asynchronous.database import AsyncDatabase
from crypto_auth import CryptoAuth
from ecc_utils import ECC_SIGNATURE_SIZE, ECC_PUBKEY_SIZE
from schemas import Event, ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, ScoreAnnounceEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from schemas import IrPacket, IrPacketRequestSchema, IrPacketObject, Station, PacketType, PACKET_HASH_LEN, IR_USERNAME_LEN
from config import Config
from hashlib import sha3_256
import inspect
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
    def event_handler(func: Callable[[Event], Awaitable[None]]) -> None:
        """
        Register a handler for a specific packet type.
        """
        # Get the event type from the function's signature.
        signature = inspect.signature(func)
        if "evt" not in signature.parameters:
            raise ValueError("Function must have a parameter named 'evt'.")
        event_type = signature.parameters["evt"].annotation

        if not issubclass(event_type, Event):
            raise ValueError("Function must accept an Event type as the first parameter.")
        
        # Register the function as a handler for the event type.
        PacketProcessor.packet_handlers[event_type] = func


    # ===== Interface to HTTP =====
    async def on_receive_packet(self, ir_packet_schema: IrPacketRequestSchema, station: Station) -> None:
        if await self.handle_acknowledgment(ir_packet_schema, station):
            # If the packet is an acknowledgment, we don't need to do anything else.
            return

        ir_packet = IrPacket(
            packet_id=ir_packet_schema.packet_id,
            data=bytes(ir_packet_schema.data),
            station_id=station.station_id,
            to_stn=False
        )

        # verify the packet
        # it would throw an exception if the packet is invalid
        await self.crypto_auth.verify_packet(ir_packet)

        event = self.parse_packet(ir_packet)
        if event is None:
            # If the packet is not a valid event, we don't need to do anything else.
            return

        hv = self.packet_hash(ir_packet)
        db_packet = IrPacketObject(packet_id=ir_packet_schema.packet_id, data=Binary(ir_packet_schema.data), hash=Binary(hv))

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

        # TODO: retransmit packets in the user queue (move these packets to station tx)


        # TODO: handle the event
        await PacketProcessor.packet_handlers.get(event.__class__, lambda x, y: None)(event, station)


    async def has_packet_for_tx(self, station: Station) -> AsyncIterator[IrPacketRequestSchema]:
        packets = self.packets.find({"_id": {"$in": station.tx}})
        async for packet in packets:
            # Convert the packet to IrPacketRequestSchema and yield it.
            yield IrPacketRequestSchema(
                packet_id=packet["packet_id"],
                data=list(packet["data"])
            )


    # ===== Interface for GameLogic =====
    async def send_packet_to_user(self, ir_packet: IrPacket, user: int) -> uuid.UUID:
        """
        Send a packet to a particular user. PacketProcessor will queue it for sending, and when activity from the given user is observed on a base station, packet will be directed to it.
        Will return immediately, and return the UUID of the packet.

        ir_packet's packet_id field can be left empty, if empty, will auto populate.
        """

        packet_id = ir_packet.packet_id or uuid.uuid4()
        ir_packet.packet_id = packet_id
        ir_packet.to_stn = True
        ir_packet.station_id = await self.get_user_last_station_uuid(user)

        if ir_packet.station_id is None:
            # If the user is not associated with any station, TODO: put to user queue
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
    def parse_packet(self, ir_packet: IrPacket) -> Optional[Event]:
        # parse against ir_packet.data using struct
        packet_type = self.get_packet_type(ir_packet)
        parsed_data = dict()
        parsed_data["packet_id"] = ir_packet.packet_id
        parsed_data["station_id"] = ir_packet.station_id
        event = None
        # ttl?
        offset = 1

        if packet_type == PacketType.kProximity:
            # Proximity packet
            parsed_data["user"] = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)
            parsed_data["signature"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN:offset+IR_USERNAME_LEN+ECC_SIGNATURE_SIZE], 'little', signed=False)
            event = ProximityEvent(**parsed_data)
        elif packet_type == PacketType.kPubAnnounce:
            # Public announce packet
            parsed_data["pubkey"] = int.from_bytes(ir_packet.data[offset:offset+ECC_PUBKEY_SIZE], 'little', signed=False)
            parsed_data["signature"] = int.from_bytes(ir_packet.data[offset+ECC_PUBKEY_SIZE:offset+ECC_PUBKEY_SIZE+ECC_SIGNATURE_SIZE], 'little', signed=False)
            event = PubAnnounceEvent(**parsed_data)
        elif packet_type == PacketType.kTwoBadgeActivity:
            # Two badge Activity packet
            parsed_data["user1"] = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)
            parsed_data["user2"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN:offset+IR_USERNAME_LEN*2], 'little', signed=False)
            parsed_data["game_data"] = ir_packet.data[offset+IR_USERNAME_LEN*2:offset+IR_USERNAME_LEN*2+5]
            parsed_data["signature"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN*2+5:offset+IR_USERNAME_LEN*2+5+ECC_SIGNATURE_SIZE], 'little', signed=False)
            return TwoBadgeActivityEvent(**parsed_data)
        elif packet_type == PacketType.kScoreAnnounce:
            # Score announce packet
            parsed_data["user"] = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)
            parsed_data["score"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN:offset+IR_USERNAME_LEN+4], 'little', signed=False)
            return ScoreAnnounceEvent(**parsed_data)
        elif packet_type == PacketType.kSingleBadgeActivity:
            # Single badge activity packet
            parsed_data["user"] = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)
            parsed_data["event_type"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN:offset+IR_USERNAME_LEN+1], 'little', signed=False)
            parsed_data["event_data"] = ir_packet.data[offset+IR_USERNAME_LEN+1:offset+IR_USERNAME_LEN+1+3]
            return SingleBadgeActivityEvent(**parsed_data)
        elif packet_type == PacketType.kSponsorActivity:
            # Sponsor activity packet
            parsed_data["user"] = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)
            parsed_data["sponsor_id"] = int.from_bytes(ir_packet.data[offset+IR_USERNAME_LEN:offset+IR_USERNAME_LEN+1], 'little', signed=False)
            parsed_data["sponsor_data"] = ir_packet.data[offset+IR_USERNAME_LEN+1:offset+IR_USERNAME_LEN+1+9]
            return SponsorActivityEvent(**parsed_data)
        else:
            # Unknown packet type
            return None

        return event


    def packet_hash(self, ir_packet: IrPacket) -> bytes:
        """
        Get the packet hash. The function will exclude the ECC signature from the hash.
        """
        return sha3_256(ir_packet.data[:-(ECC_SIGNATURE_SIZE)]).digest()[:PACKET_HASH_LEN]


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
            user = int.from_bytes(ir_packet.data[offset:offset+IR_USERNAME_LEN], 'little', signed=False)

            # update users' last station
            await self.users.update_one(
                {"user": user},
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


    async def get_user_last_station_uuid(self, user: int) -> Optional[uuid.UUID]:
        # Get the station associated with a user.
        # Should deal with roaming or multiple stations.
        # If IR received from multiple stations in a short time, we should use consider the previous station.
        # If such time is passed between two packets, we should consider them as two different packets.
        user = await self.users.find_one({"user": user})

        if user:
            return user.get("station_id")
        else:
            return None