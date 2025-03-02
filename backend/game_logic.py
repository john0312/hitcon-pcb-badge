from schemas import ActivityEvent, ProximityEvent


class GameLogic:
    def __init__(self):
        pass

    # ===== APIs for CryptoAuthLayer =====
    async def on_activity_event(evt: ActivityEvent):
        pass

    async def on_proximity_event(evt: ProximityEvent):
        pass
