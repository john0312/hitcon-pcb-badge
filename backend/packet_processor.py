from typing import Optional, AsyncIterator, Callable, Awaitable, Dict, ClassVar, Union
from bson import Binary
from crypto_auth import CryptoAuth, UnsignedPacketError
from schemas import Event
from schemas import IrPacket, IrPacketRequestSchema, IrPacketObject, Station, PacketType, PACKET_HASH_LEN
from config import Config
from hashlib import sha3_256
from database import db, redis_client
import inspect
import uuid
import typing

if typing.TYPE_CHECKING:
    import redis.asyncio as redis

from game_logic_controller import GameLogicController
from packet_parser import PacketParser

class PacketProcessor:
    packet_handlers: ClassVar[Dict[type[Event], Callable[[Event, 'PacketProcessor'], Awaitable[None]]]] = dict()

    def __init__(self, config: Config):
        self.config = config
        self.stations = db["stations"]
        self.packets = db["packets"]
        self.users = db["users"]
        self.user_queue = db["user_queue"]
        self.redis = redis_client

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
        # Bypass empty packets
        if not ir_packet_schema.data: return

        # If the packet is an acknowledgment, we don't need to do anything else.
        if await self.handle_acknowledgment(ir_packet_schema, station): return

        ir_packet = IrPacket(
            packet_id=ir_packet_schema.packet_id,
            data=bytes(ir_packet_schema.data),
            station_id=station.station_id,
            to_stn=False
        )

        event = PacketParser.parse_packet(ir_packet)
        hv = PacketProcessor.packet_hash(ir_packet)
        result = None # packet insert result

        try:
            # verify the packet
            # it would throw an exception if the packet is invalid
            # Maybe a new field in IrPacket?
            user = await CryptoAuth.verify_packet(event, ir_packet)

            # If the packet is not a valid event, we don't need to do anything else.
            if event is None: return

            db_packet = IrPacketObject(packet_id=ir_packet_schema.packet_id, data=Binary(bytes(ir_packet_schema.data)), hash=Binary(hv))

            # add the packet to the database
            result = await self.packets.insert_one(
                db_packet.model_dump(exclude={"id"})
            )

            # add packet ObjectId to station rx list
            await self.stations.update_one(
                {"station_id": station.station_id},
                {"$push": {"rx": result.inserted_id}}
            )

            # Associate the user with the station.
            if user is not None:
                await self.set_user_last_station_id(user, station.station_id)

                # retransmit packets in the user queue (move these packets to station tx)
                # Dequeue packets for the user and add them to the station tx list.
                await self.deque_user_packets(user, station)

            # handle the event
            await PacketProcessor.packet_handlers[event.__class__](event, self)
        except UnsignedPacketError as e:
            print(f"Invalid packet received: {e}")
        except AssertionError as e:
            print(f"Assertion error: {e}")
        finally:
            # Always remove rx packets from the database, in case the packet is not processible.
            if result:
                await self.stations.update_one(
                    {"station_id": station.station_id},
                    {"$pull": {"rx": result.inserted_id}}
                )
                await self.packets.delete_one({"_id": result.inserted_id})

            # Always send an acknowledgment packet.
            await self.ack(ir_packet_schema, station)


    async def has_packet_for_tx(self, station: Station) -> AsyncIterator[IrPacketRequestSchema]:
        packets = self.packets.find({"_id": {"$in": station.tx}})
        async for packet in packets:
            # Convert the packet to IrPacketRequestSchema and yield it.
            yield IrPacketRequestSchema(
                station_id=station.station_id,
                packet_id=packet["packet_id"],
                data=list(packet["data"])
            )
            await self.stations.update_one(
                {"station_id": station.station_id},
                {"$pull": {"tx": packet["_id"]}}
            )
            await self.packets.delete_one({"_id": packet["_id"]})


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
        ir_packet.station_id = await self.get_user_last_station_id(user)

        if ir_packet.station_id is None:
            # If the user is not associated with any station, put to user queue and wait until the user is associated with a station.
            return await self.queue_user_packet(ir_packet, user)
        else:
            return await self.send_packet_to_station(ir_packet)


    async def send_packet_to_station(self, ir_packet: IrPacket) -> uuid.UUID:
        hv = PacketProcessor.packet_hash(ir_packet)

        db_packet = IrPacketObject(packet_id=ir_packet.packet_id, data=Binary(ir_packet.data), hash=Binary(hv))

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
        hv = PacketProcessor.packet_hash(ir_packet)

        db_packet = IrPacketObject(packet_id=ir_packet.packet_id, data=Binary(ir_packet.data), hash=Binary(hv))

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
    @staticmethod
    def packet_hash(ir_packet: Union[IrPacket, IrPacketRequestSchema]) -> bytes:
        """
        Get the packet hash. The function will exclude the ECC signature from the hash.
        """
        data = bytes(ir_packet.data)

        return sha3_256(data).digest()[:PACKET_HASH_LEN]


    async def handle_acknowledgment(self, ir_packet: IrPacketRequestSchema, station: Station) -> bool:
        # Handle acknowledgment from the base station.
        # This is where we would update the database or perform any other necessary actions.
        packet_type = PacketParser.get_packet_type(ir_packet)
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


    async def ack(self, ir_packet: IrPacketRequestSchema, station: Station) -> None:
        # Send an acknowledgment packet to the badge through the base station.
        ack_packet = IrPacket(
            packet_id=ir_packet.packet_id,
            data=b"\x00" + PacketType.kAcknowledge.value.to_bytes(1, 'big') + PacketProcessor.packet_hash(ir_packet),
            station_id=station.station_id,
            to_stn=False
        )
        await self.send_packet_to_station(ack_packet)


    async def set_user_last_station_id(self, user: int, station_id: int) -> None:
        # Set the station associated with a user.
        # This is used to determine where to send packets for the user.
        # Use redis to store the association and automatically expire it after a certain time.
        assert self.redis is not None, "Redis connection is not initialized."
        await self.redis.set(
            f"user_station_pair:{user}", str(station_id),
            ex=self.config.get("redis", {}).get("user_station_pair_expire", 180) # default 3 minutes
        )


    async def get_user_last_station_id(self, user: int) -> Optional[int]:
        # Get the station associated with a user.
        # Should deal with roaming or multiple stations.
        # If IR received from multiple stations in a short time, we should use consider the previous station.
        # If such time is passed between two packets, we should consider them as two different packets.
        assert self.redis is not None, "Redis connection is not initialized."

        user_station_id = await self.redis.get(f"user_station_pair:{user}")
        if user_station_id is None:
            return None

        return int(user_station_id)
