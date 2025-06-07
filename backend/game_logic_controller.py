from schemas import ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from database import mongo, db
from game_logic import _GameLogic as GameLogic, GameType
import uuid

game = GameLogic(mongo)

class GameLogicController:
    # ===== APIs for PacketProcessor =====
    @staticmethod
    async def on_single_badge_activity_event(evt: SingleBadgeActivityEvent):
        # GameType
        # 0x01 - Snake
        # 0x02 - Tetris
        # 0x03 - Dino
        # 0x10 - Shake
        match evt.event_type:
            case 0x01:
                game_type = GameType.SNAKE
            case 0x02:
                game_type = GameType.TETRIS
            case 0x03:
                game_type = GameType.DINO
            case 0x10:
                game_type = GameType.SHAKE_BADGE
            case _:
                raise ValueError(f"Unknown game type: {evt.event_type}")

        await game.receive_game_score(
            player_id=evt.user,
            station_id=evt.station_id,
            event_data=evt.event_data,
            game_type=game_type,
            timestamp=evt.timestamp
        )

        return evt.user


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
        await game.attack_station(
            player_id=evt.user,
            station_id=evt.station_id,
            amount=evt.power,
            timestamp=evt.timestamp
        )

        return evt.user


    @staticmethod
    async def on_pub_announce_event(evt: PubAnnounceEvent):
        print(f"Pub announce event: {evt.pubkey.hex()}")
        print(f"Signature: {evt.signature.hex()}")
        # station <--> user has been recorded by the PacketProcessor
        pass


    @staticmethod
    async def get_user_score():
        return await game.get_game_score()


    @staticmethod
    async def get_station_score(station_id: uuid.UUID):
        return await game.get_station_score(station_id)