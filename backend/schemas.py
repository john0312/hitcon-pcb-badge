from typing import Optional, List
from pydantic import BaseModel
import uuid


class IrPacket(BaseModel):
    packet_id: Optional[uuid.UUID]
    # The packet_id to avoid duplication at base station.
    data: bytes
    station_id: int
    to_stn: bool
    # to_stn is True for backend -> base station packet, False otherwise.


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
