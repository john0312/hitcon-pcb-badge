from schemas import ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from game_logic import _GameLogic as GameLogic, GameType

game = GameLogic()

class GameLogicController:
    # ===== APIs for PacketProcessor =====
    @staticmethod
    async def on_single_badge_activity_event(evt: SingleBadgeActivityEvent):
        pass


    @staticmethod
    async def on_two_badge_activity_event(evt: TwoBadgeActivityEvent):
        pass


    @staticmethod
    async def on_game_activity_event(evt: GameActivityEvent):
        pass


    @staticmethod
    async def on_sponsor_activity_event(evt: SponsorActivityEvent):
        pass


    @staticmethod
    async def on_proximity_event(evt: ProximityEvent):
        print("Event", evt)


    @staticmethod
    async def on_pub_announce_event(evt: PubAnnounceEvent):
        pass
