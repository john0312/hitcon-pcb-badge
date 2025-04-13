from typing import Optional, List
from pydantic import BaseModel, Field, BeforeValidator
from typing import Annotated
from enum import Enum
import uuid

PyObjectId = Annotated[str, BeforeValidator(str)]
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
    kActivity = 6
    kScoreAnnounce = 7


class IrPacket(BaseModel):
    packet_id: Optional[uuid.UUID]
    # The packet_id to avoid duplication at base station.
    data: bytes
    station_id: uuid.UUID
    to_stn: bool
    # to_stn is True for backend -> base station packet, False otherwise.


# For http requests
class IrPacketRequestSchema(BaseModel):
    packet_id: Optional[uuid.UUID]
    data: bytes


# For Mongo
class IrPacketObject(BaseModel):
    # id: Optional[PyObjectId] = Field(alias="_id", default=None)
    packet_id: Optional[uuid.UUID]
    data: PyBinary
    hash: PyBinary


class ActivityEvent(BaseModel):
    event_id: Optional[uuid.UUID]
    packet_ids: List[uuid.UUID]
    players: List[int]
    signatures: List[int]
    game_data: bytes


class ProximityEvent(BaseModel):
    event_id: Optional[uuid.UUID]
    user: int
    signature: int


class Display(BaseModel):
    bar_1: str
    bar_2: str
    winning_color: str


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
    username: int
    pubkey: int
    # Tracking the last station the user was seen
    station_id: Optional[uuid.UUID]


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

# leaderboard
class ScoreEntry(BaseModel):
    name: str
    uid: int
    score: int

class ScoreBoard(BaseModel):
    scores: List[ScoreEntry]