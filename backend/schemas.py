from typing import Optional, List
from pydantic import BaseModel
import uuid


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
    station_id: uuid.UUID
    station_key: str
    display: Optional[Display]
    tx: List[IrPacket]
    rx: List[IrPacket]

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