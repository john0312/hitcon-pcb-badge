from typing import Optional, AsyncIterator, Callable, Awaitable, Dict, ClassVar, Union
from bson import Binary
from crypto_auth import CryptoAuth
from ecc_utils import ECC_SIGNATURE_SIZE, ECC_PUBKEY_SIZE
from schemas import Event, ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, ScoreAnnounceEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from schemas import IrPacket, IrPacketRequestSchema, IrPacketObject, Station, PacketType, PACKET_HASH_LEN, IR_USERNAME_LEN
from config import Config
from hashlib import sha3_256
from database import db
import inspect
import uuid
import time
from io import BytesIO

from game_logic_controller import GameLogicController

class PacketProcessor:
    packet_handlers: ClassVar[Dict[type[Event], Callable[[Event], Awaitable[None]]]] = dict()

    def __init__(self, config: Config):
        self.config = config
        self.stations = db["stations"]
        self.packets = db["packets"]
        self.users = db["users"]
        self.user_queue = db["user_queue"]

        for k, v in GameLogicController.__dict__.items():
            if k.startswith("on_") and isinstance(v, staticmethod):
                # Register the static method as an event handler
                PacketProcessor.event_handler(v.__func__)


    @staticmethod
    def event_handler(func: Callable):
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

        return func


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
        await CryptoAuth.verify_packet(ir_packet)

        event = self.parse_packet(ir_packet)
        if event is None:
            # If the packet is not a valid event, we don't need to do anything else.
            return

        hv = self.packet_hash(ir_packet)
        db_packet = IrPacketObject(packet_id=ir_packet_schema.packet_id, data=Binary(bytes(ir_packet_schema.data)), hash=Binary(hv))

        # add the packet to the database
        result = await self.packets.insert_one(
            {"$push": {"rx": db_packet.model_dump()}}
        )

        # add packet ObjectId to station rx list
        await self.stations.update_one(
            {"station_id": station.station_id},
            {"$push": {"rx": result.inserted_id}}
        )

        # TODO: retransmit packets in the user queue (move these packets to station tx)


        # TODO: handle the event
        await PacketProcessor.packet_handlers[event.__class__](event)


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
            # If the user is not associated with any station, put to user queue and wait until the user is associated with a station.
            return await self.queue_user_packet(ir_packet, user)
        else:
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


    async def queue_user_packet(self, ir_packet: IrPacket, user: int) -> uuid.UUID:
        hv = self.packet_hash(ir_packet)

        db_packet = IrPacketObject(packet_id=ir_packet.packet_id, data=Binary(ir_packet.data), hash=Binary(hv), timestamp=int(time.time()))

        # add the packet to the database
        await self.packets.insert_one(
            db_packet.model_dump(exclude={"id"})
        )

        # associate the packet with the user
        await self.user_queue.insert_one(
            {"user": user, "packet_id": ir_packet.packet_id}
        )

        return ir_packet.packet_id


    async def deque_user_packets(self, user: int, station: Station) -> Optional[list[uuid.UUID]]:
        # Dequeue all packets for the user.
        # This is where we would update the database or perform any other necessary actions.
        packet_ids_query = await self.user_queue.find({"user": user}).to_list()
        packet_ids = [packet["packet_id"] for packet in packet_ids_query]
        if not packet_ids:
            return None

        # Add the packets to the station tx list.
        packet_objects_query = await self.packets.find({"packet_id": {"$in": packet_ids}}).to_list()
        await self.stations.update_one(
            {"station_id": station.station_id},
            {"$push": {"tx": {"$each": [packet["_id"] for packet in packet_objects_query]}}}
        )

        await self.user_queue.delete_many({"packet_id": {"$in": packet_ids}})

        return packet_ids


    # ===== Internal methods =====
    def parse_packet(self, ir_packet: IrPacket) -> Optional[Event]:
        packet_type = self.get_packet_type(ir_packet)
        packet_id = ir_packet.packet_id
        station_id = ir_packet.station_id

        buf = BytesIO(ir_packet.data)
        buf.read(1)  # Skip TTL
        buf.read(1)  # Skip the first byte (packet type)
        b2i = lambda x: int.from_bytes(x, 'little', signed=False)

        match packet_type:
            case PacketType.kProximity:
                # Proximity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                power = b2i(buf.read(1))
                nonce = b2i(buf.read(2))
                signature = b2i(buf.read(ECC_SIGNATURE_SIZE))
                return ProximityEvent(packet_id=packet_id, station_id=station_id, user=user, power=power, nonce=nonce, signature=signature)

            case PacketType.kPubAnnounce:
                # Public announce packet
                pubkey = b2i(buf.read(ECC_PUBKEY_SIZE))
                signature = b2i(buf.read(ECC_SIGNATURE_SIZE))
                return PubAnnounceEvent(packet_id=packet_id, station_id=station_id, pubkey=pubkey, signature=signature)

            case PacketType.kTwoBadgeActivity:
                # Two badge Activity packet
                user1 = b2i(buf.read(IR_USERNAME_LEN))
                user2 = b2i(buf.read(IR_USERNAME_LEN))
                game_data = buf.read(5)
                signature = b2i(buf.read(ECC_SIGNATURE_SIZE))
                return TwoBadgeActivityEvent(packet_id=packet_id, station_id=station_id, user1=user1, user2=user2, game_data=game_data, signature=signature)

            case PacketType.kScoreAnnounce:
                # Score announce packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                score = b2i(buf.read(4))
                return ScoreAnnounceEvent(packet_id=packet_id, station_id=station_id, user=user, score=score)

            case PacketType.kSingleBadgeActivity:
                # Single badge activity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                event_type = b2i(buf.read(1))
                event_data = buf.read(3)
                return SingleBadgeActivityEvent(packet_id=packet_id, station_id=station_id, user=user, event_type=event_type, event_data=event_data)

            case PacketType.kSponsorActivity:
                # Sponsor activity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                sponsor_id = b2i(buf.read(1))
                sponsor_data = buf.read(9)
                return SponsorActivityEvent(packet_id=packet_id, station_id=station_id, user=user, sponsor_id=sponsor_id, sponsor_data=sponsor_data)

            case _:
                # Unknown packet type
                return None


    def packet_hash(self, ir_packet: Union[IrPacket, IrPacketRequestSchema]) -> bytes:
        """
        Get the packet hash. The function will exclude the ECC signature from the hash.
        """
        data = bytes(ir_packet.data[:-(ECC_SIGNATURE_SIZE)])
        return sha3_256(data).digest()[:PACKET_HASH_LEN]


    async def handle_acknowledgment(self, ir_packet: IrPacketRequestSchema, station: Station) -> bool:
        # Handle acknowledgment from the base station.
        # This is where we would update the database or perform any other necessary actions.
        packet_type = self.get_packet_type(ir_packet)
        if packet_type == PacketType.kAcknowledge:
            hv = ir_packet.data[1:1+PACKET_HASH_LEN]

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


    def get_packet_type(self, ir_packet: Union[IrPacket, IrPacketRequestSchema]) -> Optional[PacketType]:
        # Determine the type of packet based on its contents.
        # This is a placeholder implementation and should be replaced with actual logic.
        raw_type = ir_packet.data[1]
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
        user_object = await self.users.find_one({"user": user})

        if user_object:
            return user_object.get("station_id")
        else:
            return None
