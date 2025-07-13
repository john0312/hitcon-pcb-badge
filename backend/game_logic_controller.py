from schemas import PacketType, ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, SingleBadgeActivityEvent, SponsorActivityEvent, IrPacket
from database import mongo, db, redis_client
from game_logic import _GameLogic as GameLogic, GameType
from ecc_utils import ECC_SIGNATURE_SIZE

# Simply for type notation
import typing
if typing.TYPE_CHECKING:
    from packet_processor import PacketProcessor

game = GameLogic(mongo, redis_client)

class GameLogicController:
    # ===== APIs for PacketProcessor =====
    @staticmethod
    async def on_single_badge_activity_event(evt: SingleBadgeActivityEvent, packet_processor: 'PacketProcessor'):
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

        # Parse score from event data
        score = int.from_bytes(evt.event_data, 'big')
        score = (score & 0xFFF000) >> 12  # Extract the score from the event data

        # TODO: check nonce

        await game.receive_game_score_single_player(
            player_id=evt.user,
            station_id=evt.station_id,
            score=score,
            game_type=game_type,
            timestamp=evt.timestamp
        )


    @staticmethod
    async def on_two_badge_activity_event(evt: TwoBadgeActivityEvent, packet_processor: 'PacketProcessor'):
        # game_data structure (MSB):
        # Bit [0:4] - Game Type
        #          0x00 - None/Reserved
        #          0x01 - Snake
        #          0x02 - Tetris
        # Bit [4:14] - Player 1 Score
        # Bit [14:24] - Player 2 Score
        # Bit [24:40] - Nonce
        data_bits = f"{evt.game_data:0>40b}"
        raw_game_type = int(data_bits[0:4], 2)
        player1_score = int(data_bits[4:14], 2)
        player2_score = int(data_bits[14:24], 2)
        nonce = int(data_bits[24:40], 2)

        match raw_game_type:
            case 0x01:
                game_type = GameType.SNAKE
            case 0x02:
                game_type = GameType.TETRIS
            case 0x03:
                game_type = GameType.DINO
            case _:
                raise ValueError(f"Unknown game type: {raw_game_type}")

        scores = [(evt.user1, player1_score), (evt.user2, player2_score)]
        scores.sort(key=lambda x: x[0])

        queue = db["battle_queue"]
        # Check if the game is already in the queue
        existing_game = await queue.find_one({
            "game_type": str(game_type),
            "player1": scores[0][0],
            "player2": scores[1][0],
            "score1": scores[0][1],
            "score2": scores[1][1],
            "nonce": nonce
        })

        if existing_game:
            game_event = GameActivityEvent(
                event_id=evt.event_id, # use second packet's event_id
                station_id=evt.station_id,
                game_type_str=str(game_type),
                user1=scores[0][0],
                user2=scores[1][0],
                score1=scores[0][1],
                score2=scores[1][1],
                nonce=nonce,
                timestamp=evt.timestamp,
                signatures=[evt.signature, existing_game["signature"]]  # TODO: Combine signatures in right order
            )
            await GameLogicController.on_game_activity_event(game_event)
            await queue.delete_one({"_id": existing_game["_id"]})
        else:
            await queue.insert_one({
                "game_id": evt.event_id,
                "game_type": str(game_type),
                "packet_id": evt.packet_id,
                "player1": scores[0][0],
                "player2": scores[1][0],
                "score1": scores[0][1],
                "score2": scores[1][1],
                "nonce": nonce,
                "signature": evt.signature
            })


    @staticmethod
    async def on_game_activity_event(evt: GameActivityEvent, packet_processor: 'PacketProcessor'):
        await game.receive_game_score_two_player(
            two_player_event_id=evt.event_id,
            player1_id=evt.user1,
            player2_id=evt.user2,
            station_id=evt.station_id,
            score1=evt.score1,
            score2=evt.score2,
            game_type=GameType(evt.game_type_str),
        )


    @staticmethod
    async def on_sponsor_activity_event(evt: SponsorActivityEvent, packet_processor: 'PacketProcessor'):
        # TODO: pre-generate sponsor random hash
        pass


    @staticmethod
    async def on_proximity_event(evt: ProximityEvent, packet_processor: 'PacketProcessor'):
        await game.attack_station(
            player_id=evt.user,
            station_id=evt.station_id,
            amount=evt.power,
            timestamp=evt.timestamp
        )

        user_score = await game.get_game_score(player_id=evt.user)
        # announce the score to the user
        pkt = IrPacket(
            data=b"".join([
                b"\x00",                                    # TTL
                bytes([PacketType.kScoreAnnounce.value]),   # PacketType
                evt.user.to_bytes(4, 'little'),             # User
                user_score.to_bytes(4, 'little'),           # Score
                b"\x87" * ECC_SIGNATURE_SIZE,               # TODO: Dummy signature 
            ]),
            station_id=evt.station_id,
            to_stn=True
        )
        await packet_processor.send_packet_to_user(pkt, evt.user)


    @staticmethod
    async def on_pub_announce_event(evt: PubAnnounceEvent, packet_processor: 'PacketProcessor'):
        print(f"Pub announce event: {evt.pubkey.hex()}")
        print(f"Signature: {evt.signature.hex()}")
        # station <--> user has been recorded by the PacketProcessor
        result = await db["users"].find_one({"pubkey": evt.pubkey})  # Ensure user exists


    @staticmethod
    async def get_user_score():
        return await game.get_game_score()


    @staticmethod
    async def get_station_score(station_id: int):
        return await game.get_station_score(station_id=station_id)
