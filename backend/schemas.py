from typing import Optional, List, Any, Callable
from pydantic import BaseModel, Field, BeforeValidator
from pydantic_core import core_schema
from bson import ObjectId
from typing import Annotated
from enum import Enum
import uuid
import datetime
from typing import Dict

utcnow = lambda: datetime.datetime.now(datetime.timezone.utc)

class _ObjectIdPydanticAnnotation:
    # Based on https://docs.pydantic.dev/latest/usage/types/custom/#handling-third-party-types.

    @classmethod
    def __get_pydantic_core_schema__(
        cls,
        _source_type: Any,
        _handler: Callable[[Any], core_schema.CoreSchema],
    ) -> core_schema.CoreSchema:
        def validate_from_str(input_value: str) -> ObjectId:
            return ObjectId(input_value)

        return core_schema.union_schema(
            [
                # check if it's an instance first before doing any further work
                core_schema.is_instance_schema(ObjectId),
                core_schema.no_info_plain_validator_function(validate_from_str),
            ],
            serialization=core_schema.to_string_ser_schema(),
        )

PyObjectId = Annotated[ObjectId, _ObjectIdPydanticAnnotation]
PyBinary = Annotated[bytes, BeforeValidator(bytes)]

PACKET_HASH_LEN = 6
IR_USERNAME_LEN = 4

class PacketType(Enum):
    kGame = 0  # disabled
    kShow = 1
    kTest = 2
    # Packet types for 2025
    kAcknowledge = 3
    kProximity = 4
    kPubAnnounce = 5
    kTwoBadgeActivity = 6
    kScoreAnnounce = 7
    kSingleBadgeActivity = 8
    kSponsorActivity = 9


class IrPacket(BaseModel):
    packet_id: Optional[uuid.UUID] = Field(None)
    # The packet_id to avoid duplication at base station.
    data: bytes
    station_id: Optional[uuid.UUID] = Field(None)
    to_stn: bool
    # to_stn is True for backend -> base station packet, False otherwise.


# For http requests
class IrPacketRequestSchema(BaseModel):
    packet_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    data: List[int]


class Display(BaseModel):
    bar_1: str
    bar_2: str
    winning_color: str


# Leaderboard
class ScoreEntry(BaseModel):
    name: str
    uid: int
    scores: Dict[str, int]  
    total_score: int


class ScoreBoard(BaseModel):
    scores: List[ScoreEntry]


# For Mongo
class IrPacketObject(BaseModel):
    # id: Optional[PyObjectId] = Field(alias="_id", default=None)
    packet_id: Optional[uuid.UUID] = Field(None)
    data: PyBinary
    hash: PyBinary
    timestamp: Optional[datetime.datetime] = Field(default_factory=utcnow)


# === Events from Parsed packets ===
class Event(BaseModel):
    event_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    packet_id: Optional[uuid.UUID] = Field(None)
    station_id: Optional[uuid.UUID] = Field(None)
    timestamp: Optional[datetime.datetime] = Field(default_factory=utcnow)


class ProximityEvent(Event):
    user: int
    power: int
    nonce: int
    signature: int


class PubAnnounceEvent(Event):
    pubkey: int
    signature: int


# Converted from user packet
class TwoBadgeActivityEvent(Event):
    user1: int
    user2: int
    game_data: bytes
    signature: int


# Collected ActivityEvent from two users
class GameActivityEvent(Event):
    packet_ids: List[uuid.UUID]
    players: List[int]
    signatures: List[int]
    game_data: bytes


class ScoreAnnounceEvent(Event):
    user: int
    score: int


class SingleBadgeActivityEvent(Event):
    user: int
    event_type: int
    event_data: bytes


class SponsorActivityEvent(Event):
    user: int
    sponsor_id: int
    sponsor_data: bytes


# For Mongo collections `stations`
class Station(BaseModel):
    id: Optional[PyObjectId] = Field(alias="_id", default=None)
    station_id: uuid.UUID
    station_key: str
    display: Optional[Display]
    tx: List[PyObjectId]
    rx: List[PyObjectId]

    class Config:
        json_encoders = {
            uuid.UUID: str
        }


# For Mongo collections `users`
class User(BaseModel):
    id: Optional[PyObjectId] = Field(alias="_id", default=None)
    user: int
    pubkey: int
    # Tracking the last station the user was seen
    station_id: Optional[uuid.UUID] = Field(None)


# Elliptic Curve Crytography related.
# Curve is hardcoded.
class EccPoint(BaseModel):
    x: int
    y: int

class EccPublicKey(BaseModel):
    point: EccPoint

class EccSignature(BaseModel):
    pub: Optional[EccPublicKey]
    r: int
    s: int

class EccPrivateKey(BaseModel):
    dA: int
