from schemas import utcnow, PacketType, ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, ScoreAnnounceEvent, SingleBadgeActivityEvent, SponsorActivityEvent, IrPacket, ReCTFSolves
from config import Config
from database import mongo, db, redis_client
from game_logic import _GameLogic as GameLogic, GameType, Constants
from ecc_utils import ECC_SIGNATURE_SIZE
from crypto_auth import CryptoAuth
from badge_link_controller import BadgeLinkController

# Simply for type notation
import typing
if typing.TYPE_CHECKING:
    from packet_processor import PacketProcessor

config = Config("config.yaml")
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
        # event_data[3]
        # packet.event_data[0] = (myScore & 0xFF);
        # packet.event_data[1] = (myScore & 0x0300) >> 8;
        # packet.event_data[1] |= (data.nonce & 0x03F) << 2;
        # packet.event_data[2] = (data.nonce & 0x3FC0) >> 6;
        score = (evt.event_data[0] | ((evt.event_data[1] & 0x03) << 8)) & 0x3FF
        nonce = (evt.event_data[1] >> 2) | (evt.event_data[2] << 6)

        # check nonce
        nonce_key = f"single_badge:{evt.user}:{game_type}:{nonce}"
        if (await redis_client.get(nonce_key)) is not None:
            # Duplicate event, ignore it
            return

        await redis_client.set(nonce_key, "1", ex=config.get("redis", {}).get("game_nonce_expire", 180)) # default 3 minutes
        await game.receive_game_score_single_player(
            player_id=evt.user,
            station_id=evt.station_id // 10,
            score=score,
            game_type=game_type,
            timestamp=evt.timestamp
        )

        await GameLogicController.score_announce(
            user=evt.user,
            packet_processor=packet_processor
        )

        # attack station
        team = await CryptoAuth.get_user_team(evt.user)
        await game.attack_station(
            player_id=evt.user,
            station_id=evt.station_id // 10,
            amount=team * score,
            timestamp=evt.timestamp
        )


    @staticmethod
    async def on_two_badge_activity_event(evt: TwoBadgeActivityEvent, packet_processor: 'PacketProcessor'):
        # game_data structure (MSB):
        # game_data[5]
        # // Game Type: byte 0 bits 0:4
        # packet.game_data[0] = data.gameType & 0xf;
        # // Player 1 Score: byte 0 bits 4:8, byte 1 bits 0:6
        # packet.game_data[0] |= (data.myScore & 0xf) << 4;
        # packet.game_data[1] = (data.myScore & 0x3f0) >> 4;
        # // Player 2 Score: byte 1 bits 6:8, byte 2 bits 0:8
        # packet.game_data[1] |= (data.otherScore & 0x3) << 6;
        # packet.game_data[2] = (data.otherScore & 0x3fc) >> 2;
        # // Nonce: byte 3, byte 4
        raw_game_type = evt.game_data[0] & 0x0F
        player1_score = ((evt.game_data[0] & 0xF0) >> 4) | ((evt.game_data[1] & 0x3F) << 4)
        player2_score = ((evt.game_data[1] & 0xC0) >> 6) | ((evt.game_data[2] & 0xFF) << 2)
        nonce = int.from_bytes(evt.game_data[3:5], 'little', signed=False)

        match raw_game_type:
            case 0x01:
                game_type = GameType.SNAKE
            case 0x02:
                game_type = GameType.TETRIS
            case 0x03:
                game_type = GameType.DINO
            case 0x4:
                game_type = GameType.TAMA
            case _:
                raise ValueError(f"TwoBadgeGameActivity: Unknown game type: {raw_game_type}")

        if evt.user1 > evt.user2:
            scores = [(evt.user2, player2_score), (evt.user1, player1_score)]
            # user1 and user2 inversed, should adjust packet_from
            evt.packet_from = 2 if evt.packet_from == 1 else 1
        elif evt.user1 < evt.user2:
            scores = [(evt.user1, player1_score), (evt.user2, player2_score)]
        else:
            # the equal case should not happen
            raise ValueError(f"TwoBadgeGameActivity: users are equal {evt.user1} == {evt.user2}")

        queue = db["battle_queue"]
        # Match TwoBadgeActivityPacket from both users
        existing_game = await queue.find_one({
            "game_type": str(game_type),
            "player1": scores[0][0],
            "player2": scores[1][0],
            "score1": scores[0][1],
            "score2": scores[1][1],
            "nonce": nonce
        })

        if existing_game:
            if existing_game["packet_from"] == evt.packet_from:
                # This is a duplicate packet, ignore it
                return

            if evt.packet_from == 1:
                packet_ids = [evt.packet_id, existing_game["packet_id"]]
                signatures = [evt.signature, existing_game["signature"]]
            else:
                packet_ids = [existing_game["packet_id"], evt.packet_id]
                signatures = [existing_game["signature"], evt.signature]

            game_event = GameActivityEvent(
                packet_id=evt.packet_id,
                packet_ids=packet_ids,
                event_id=evt.event_id, # use second packet's event_id
                station_id=evt.station_id,
                game_type_str=str(game_type),
                user1=scores[0][0],
                user2=scores[1][0],
                score1=scores[0][1],
                score2=scores[1][1],
                nonce=nonce,
                timestamp=evt.timestamp,
                signatures=signatures
            )
            await GameLogicController.on_game_activity_event(game_event, packet_processor)
            await queue.delete_one({"_id": existing_game["_id"]})
        else:
            # Insert the game into the queue
            await queue.insert_one({
                "game_id": evt.event_id,
                "game_type": str(game_type),
                "packet_id": evt.packet_id,
                "player1": scores[0][0],
                "player2": scores[1][0],
                "score1": scores[0][1],
                "score2": scores[1][1],
                "nonce": nonce,
                "signature": evt.signature,
                "packet_from": evt.packet_from
            })


    @staticmethod
    async def on_game_activity_event(evt: GameActivityEvent, packet_processor: 'PacketProcessor'):
        print(f"Game activity event: {evt.game_type_str}, {evt.user1} vs {evt.user2}, scores: {evt.score1} vs {evt.score2}, nonce: {evt.nonce}")

        # Check nonce
        nonce_key = f"game_activity:{evt.user1}:{evt.user2}:{evt.game_type_str}:{evt.nonce}"
        if (await redis_client.get(nonce_key)) is not None:
            # Duplicate event, ignore it
            return
        await redis_client.set(nonce_key, "1", ex=config.get("redis", {}).get("game_nonce_expire", 180)) # default 3 minutes

        await game.receive_game_score_two_player(
            two_player_event_id=evt.event_id,
            player1_id=evt.user1,
            player2_id=evt.user2,
            station_id=evt.station_id // 10,
            score1=evt.score1,
            score2=evt.score2,
            game_type=GameType(evt.game_type_str),
            timestamp=evt.timestamp,
            log_only=(evt.game_type_str == GameType.TAMA) # TAMA is a log-only game, no score update
        )

        await GameLogicController.score_announce(
            user=evt.user1,
            packet_processor=packet_processor
        )

        await GameLogicController.score_announce(
            user=evt.user2,
            packet_processor=packet_processor
        )

        # attack station
        team1 = await CryptoAuth.get_user_team(evt.user1)
        await game.attack_station(
            player_id=evt.user1,
            station_id=evt.station_id // 10,
            amount=team1 * evt.score1,
            timestamp=evt.timestamp
        )

        team2 = await CryptoAuth.get_user_team(evt.user2)
        await game.attack_station(
            player_id=evt.user2,
            station_id=evt.station_id // 10,
            amount=team2 * evt.score2,
            timestamp=evt.timestamp
        )


    @staticmethod
    async def on_sponsor_activity_event(evt: SponsorActivityEvent, packet_processor: 'PacketProcessor'):
        print(f"Sponsor activity event: {evt.sponsor_id}, user: {evt.user}, nonce: {evt.nonce}")

        # Check nonce
        nonce_key = f"sponsor_activity:{evt.user}:{evt.sponsor_id}:{evt.nonce}"
        if (await redis_client.get(nonce_key)) is not None:
            # Duplicate event, ignore it
            return
        await redis_client.set(nonce_key, "1", ex=config.get("redis", {}).get("game_nonce_expire", 180))
        # TODO: do not add score from one sponsor twice

        await game.receive_game_score_single_player(
            player_id=evt.user,
            station_id=evt.station_id // 10,
            score=Constants.SPONSOR_CONNECT_SCORE,
            sponsor_id=evt.sponsor_id,
            game_type=GameType.CONNECT_SPONSOR,
            timestamp=evt.timestamp
        )

        await GameLogicController.score_announce(
            user=evt.user,
            packet_processor=packet_processor
        )


    @staticmethod
    async def on_proximity_event(evt: ProximityEvent, packet_processor: 'PacketProcessor'):
        power = evt.power + 1

        await game.receive_game_score_single_player(
            player_id=evt.user,
            station_id=evt.station_id // 10,
            score=power,
            game_type=GameType.SHAKE_BADGE,
            timestamp=evt.timestamp
        )

        team = await CryptoAuth.get_user_team(evt.user) # team is in fact sign, positive for team RED (?), negative for team Blue (?)

        await game.attack_station(
            player_id=evt.user,
            station_id=evt.station_id // 10,
            amount=team * power,
            timestamp=evt.timestamp
        )

        # announce the score to the user
        await GameLogicController.score_announce(
            user=evt.user,
            packet_processor=packet_processor
        )


    @staticmethod
    async def on_pub_announce_event(evt: PubAnnounceEvent, packet_processor: 'PacketProcessor'):
        print(f"Pub announce event: {evt.pubkey.hex()}")
        print(f"Signature: {evt.signature.hex()}")
        # station <--> user has been recorded by the PacketProcessor
        pub_x = CryptoAuth.encode_pubkey(evt.pubkey)

        # Check if the user already exists
        existing_user = await db["users"].find_one({"pubkey": pub_x})
        if not existing_user:
            # Create a new user
            user = await CryptoAuth.create_user(evt.pubkey)
            print(f"New user created: {user}")
        else:
            user = existing_user["user"]
            print(f"Existing user: {user}")


    @staticmethod
    async def on_score_announce_event(evt: ScoreAnnounceEvent, packet_processor: 'PacketProcessor'):
        pass


    @staticmethod
    async def get_user_score(user: int):
        return await game.get_game_score(player_id=user)


    @staticmethod
    async def get_user_score_history(user: int):
        return await game.get_game_history(player_id=user)


    @staticmethod
    async def get_user_scoreboard():
        # TODO: fetching scoreboard from GameLogic
        pass


    @staticmethod
    async def get_station_score(station_id: int):
        return await game.get_station_score(station_id=station_id // 10)


    @staticmethod
    async def get_station_score_history(station_id: int):
        return await game.get_station_attack_history(station_id=station_id // 10)


    @staticmethod
    async def get_stations_scores():
        # TODO: fetching stations scores from GameLogic
        pass


    @staticmethod
    async def apply_rectf_score(uid: str, user: int = None, solves: typing.Optional[ReCTFSolves] = None):
        """
        Apply ReCTF score as buff to the user.
        If user is None (aka not linked with a badge), it will store the score in the database for later application.
        If solves is None, it will try to fetch the previous known ReCTF score from the database.
        """

        # postpone if the user has not bind the badge username
        if user is None and solves is not None:
            await db["unapplied_rectf_scores"].insert_one({
                "uid": uid,
                "solves": solves.model_dump(),
                "timestamp": utcnow()
            })

        if user is not None:
            if solves is not None:
                # apply the buff with received ReCTF score
                await game.update_player_buff(
                    player_id=user,
                    buff_a=solves.a,
                    buff_b=solves.b,
                    timestamp=utcnow()
                )
            else:
                # apply the previous known ReCTF score
                result = await db["unapplied_rectf_scores"].find_one({"uid": uid})
                if result:
                    solves = ReCTFSolves(**result["solves"])
                    await game.update_player_buff(
                        player_id=user,
                        buff_a=solves.a,
                        buff_b=solves.b,
                        timestamp=result["timestamp"]
                    )
                    await db["unapplied_rectf_scores"].delete_one({"uid": uid})
                else:
                    print(f"No ReCTF score has received for UID: {uid}, user: {user}")


    @staticmethod
    async def score_announce(user: int, packet_processor: 'PacketProcessor'):
        """
        Announce the score to the user.
        This is used for the ReCTF score announcement.
        """
        user_score = await game.get_game_score(player_id=user)
        # announce the score to the user
        pkt = IrPacket(
            data=b"".join([
                b"\x00",                                    # TTL
                bytes([PacketType.kScoreAnnounce.value]),   # PacketType
                user.to_bytes(4, 'little'),             # User
                user_score.to_bytes(4, 'little'),           # Score
            ]),
            to_stn=True
        )
        signed_pkt = CryptoAuth.sign_packet(pkt)
        await packet_processor.send_packet_to_user(signed_pkt, user)
