from packet_processor import PacketProcessor
from schemas import ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from config import Config
from pymongo.asynchronous.database import AsyncDatabase


class GameLogic:
    def __init__(self, config: Config, packet_processor: PacketProcessor, db: AsyncDatabase):
        self.config = config
        self.packet_processor = packet_processor
        self.stations = db["stations"]
        self.users = db["users"]
        self.events = db["events"]


    # ===== APIs for PacketProcessor =====
    @PacketProcessor.event_handler
    async def on_single_badge_activity_event(evt: SingleBadgeActivityEvent):
        pass


    @PacketProcessor.event_handler
    async def on_two_badge_activity_event(evt: TwoBadgeActivityEvent):
        pass


    @PacketProcessor.event_handler
    async def on_game_activity_event(evt: GameActivityEvent):
        pass


    @PacketProcessor.event_handler
    async def on_sponsor_activity_event(evt: SponsorActivityEvent):
        pass


    @PacketProcessor.event_handler
    async def on_proximity_event(evt: ProximityEvent):
        pass


    @PacketProcessor.event_handler
    async def on_pub_announce_event(evt: PubAnnounceEvent):
        pass
