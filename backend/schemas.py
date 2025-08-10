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
TAMA_DATA_LEN = 6
MESSAGE_LEN = 24

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
    kShowMsg = 10
    kRequestScore = 11
    kSavePet = 12
    kRestorePet = 13


class IrPacket(BaseModel):
    packet_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    # The packet_id to avoid duplication at base station.
    data: bytes
    station_id: Optional[int] = Field(0)
    to_stn: bool
    # to_stn is True for backend -> base station packet, False otherwise.


# For http requests
class IrPacketRequestSchema(BaseModel):
    station_id: Optional[int] = Field(0)
    packet_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    data: List[int]


class Display(BaseModel):
    bar_1: str
    bar_2: str
    winning_color: str


# Leaderboard
class ScoreEntry(BaseModel):
    name: Optional[str] = ""
    player_id: int
    scores: Dict[str, int]
    total_score: int
    connected_sponsors: List[int]


class ScoreBoard(BaseModel):
    scores: List[ScoreEntry]


# For Mongo
class IrPacketObject(BaseModel):
    # id: Optional[PyObjectId] = Field(alias="_id", default=None)
    packet_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    data: PyBinary
    hash: PyBinary
    timestamp: Optional[datetime.datetime] = Field(default_factory=utcnow)


# === Events from Parsed packets ===
class Event(BaseModel):
    event_id: Optional[uuid.UUID] = Field(default_factory=uuid.uuid4)
    packet_id: Optional[uuid.UUID] = Field(None)
    station_id: Optional[int] = Field(0)
    timestamp: Optional[datetime.datetime] = Field(default_factory=utcnow)


class ProximityEvent(Event):
    user: int
    power: int
    nonce: int
    signature: bytes


class PubAnnounceEvent(Event):
    pubkey: bytes
    signature: bytes


# Converted from user packet
class TwoBadgeActivityEvent(Event):
    user1: int
    user2: int
    game_data: bytes
    signature: bytes
    packet_from: Optional[int] = Field(0) # from user 1 or user 2, this would be set by CryptoAuth


# Collected ActivityEvent from two users
class GameActivityEvent(Event):
    packet_ids: List[uuid.UUID]
    game_type_str: str
    user1: int
    user2: int
    score1: int
    score2: int
    nonce: int
    signatures: List[bytes]


class ScoreAnnounceEvent(Event):
    user: int
    score: int
    signature: bytes


class SingleBadgeActivityEvent(Event):
    user: int
    event_type: int
    event_data: bytes
    signature: bytes


class SponsorActivityEvent(Event):
    user: int
    sponsor_id: int
    nonce: int
    signature: bytes


class ShowMsgEvent(Event):
    user: int
    msg: bytes


class RequestScoreEvent(Event):
    user: int


class SavePetEvent(Event):
    user: int
    pet_data: bytes
    signature: bytes


class RestorePetEvent(Event):
    user: int
    pet_data: bytes
    signature: bytes


# For Mongo collections `stations`
class Station(BaseModel):
    id: Optional[PyObjectId] = Field(alias="_id", default=None)
    station_id: int
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
    pet_data: bytes


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

    @staticmethod
    def from_bytes(raw_sig: bytes, pub: Optional[EccPublicKey] = None) -> 'EccSignature':
        if len(raw_sig) != 14: # ECC_SIGNATURE_SIZE
            raise ValueError(f"Invalid signature length: {len(raw_sig)}")
        r = int.from_bytes(raw_sig[:7], 'little', signed=False)
        s = int.from_bytes(raw_sig[7:], 'little', signed=False)

        return EccSignature(r=r, s=s, pub=pub)

    def to_bytes(self) -> bytes:
        return self.r.to_bytes(7, 'little', signed=False) + self.s.to_bytes(7, 'little', signed=False)

class EccPrivateKey(BaseModel):
    dA: int


## ReCTF related
class ReCTFSolves(BaseModel):
    a: int = Field(0)
    b: int = Field(0)


class ReCTFScoreSchema(BaseModel):
    uid: str
    solves: ReCTFSolves


## Badge Linking related
class BadgeLinkSchema(BaseModel):
    badge_user: int
    name: Optional[str]
