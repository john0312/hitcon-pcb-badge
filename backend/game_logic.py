from schemas import ActivityEvent, ProximityEvent
from config import Config


class GameLogic:
    def __init__(self, config: Config):
        self.config = config

    # ===== APIs for CryptoAuthLayer =====
    async def on_activity_event(evt: ActivityEvent):
        pass

    async def on_proximity_event(evt: ProximityEvent):
        pass
