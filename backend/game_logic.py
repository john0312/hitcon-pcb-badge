import asyncio
from dataclasses import dataclass, fields
from datetime import datetime, timedelta
import pymongo

try:
    from enum import StrEnum
except ImportError:
    from enum import Enum
    class StrEnum(str, Enum):
        def __str__(self):
            return str(self.value)

        def __repr__(self):
            return f"{self.__class__.__name__}.{self.name}"


class GameType(StrEnum):
    SHAKE_BADGE = "shake_badge"
    DINO = "dino"
    SNAKE = "snake"
    TETRIS = "tetris"
    CONNECT_SPONSOR = "connect_sponsor"
    RECTF = "rectf"


@dataclass
class Constants:
    DATABASE_NAME: str = "game_logic"
    ATTACK_HISTORY_COLLECTION: str = "attack_history"
    SCORE_HISTORY_COLLECTION: str = "score_history"

    STATION_SCORE_LB: int = -1000
    STATION_SCORE_UB: int = 1000
    STATION_NEUTRAL_LB: int = -300
    STATION_NEUTRAL_UB: int = 300

    STATION_SCORE_DECAY_INTERVAL: int = 30 # seconds
    STATION_SCORE_DECAY_AMOUNT: int = 10

    def reset(self):
        for field in fields(self):
            setattr(self, field.name, field.default)


const = Constants()


def clamp(value: int, min_value: int, max_value: int) -> int:
    """
    Clamp the value between min_value and max_value.
    """
    return max(min(value, max_value), min_value)


def sign(value: int) -> int:
    """
    Return the sign of the value.
    """
    if value > 0:
        return 1
    elif value < 0:
        return -1
    else:
        return 0


class _GameLogic:
    def __init__(self, mongo_client: pymongo.AsyncMongoClient, start_time: datetime = None):
        # TODO: maybe use config to update constants
        self.db = mongo_client[const.DATABASE_NAME]
        self.attack_history = self.db[const.ATTACK_HISTORY_COLLECTION]
        self.score_history = self.db[const.SCORE_HISTORY_COLLECTION]

        if start_time is None:
            start_time = datetime.now()
        self.start_time = start_time

    async def clear_database(self):
        await self.attack_history.delete_many({})
        await self.score_history.delete_many({})

    async def attack_station(self, player_id: int, station_id: int, amount: int, timestamp: datetime):
        """
        This method is called when a player attacks a station.
        This will add a record in the database. The station's score will be calculated
        based on the history of attacks.
        """
        # TODO: validate the player_id and the amount
        await self.attack_history.insert_one({
            "player_id": player_id,
            "station_id": station_id,
            "amount": amount,
            "timestamp": timestamp,
        })

    async def get_station_score_history(self, *, player_id: int = None, station_id: int = None, before: datetime = None):
        if before is None:
            before = datetime.now()

        query = {"timestamp": {"$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        cursor = self.attack_history.find(query).sort("timestamp", pymongo.ASCENDING)

        async for record in cursor:
            yield record

    async def get_station_score(self, *, player_id: int = None, station_id: int = None, before: datetime = None) -> int:
        # TODO: cache the results
        if before is None:
            before = datetime.now()

        total_score = 0
        time_pointer = self.start_time

        def proceed(until: datetime):
            nonlocal time_pointer
            nonlocal total_score
            # Decay the score based on the time passed
            while time_pointer <= until:
                time_pointer += timedelta(seconds=const.STATION_SCORE_DECAY_INTERVAL)
                total_score += -1 * sign(total_score) * min(const.STATION_SCORE_DECAY_AMOUNT, abs(total_score))

        async for record in self.get_station_score_history(player_id=player_id, station_id=station_id, before=before):
            proceed(record["timestamp"])
            total_score = clamp(total_score + record["amount"], const.STATION_SCORE_LB, const.STATION_SCORE_UB)

        proceed(before)
        return total_score

    async def receive_game_score(self, player_id: int, station_id: int, score: int, game_type: GameType, timestamp: datetime):
        match game_type:
            case GameType.SHAKE_BADGE:
                # TODO: validate the score and timestamp
                pass

            case GameType.DINO:
                # TODO: validate the score and timestamp
                pass

            case GameType.SNAKE:
                # TODO: validate the score and timestamp
                pass

            case GameType.TETRIS:
                # TODO: validate the score and timestamp
                pass

            case GameType.CONNECT_SPONSOR:
                # TODO: validate the score and timestamp
                pass

            case GameType.RECTF:
                # TODO: validate the score and timestamp
                pass

        await self.score_history.insert_one({
            "player_id": player_id,
            "station_id": station_id,
            "score": score,
            "game_type": game_type,
            "timestamp": timestamp,
        })

    async def get_game_history(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, before: datetime = None):
        if before is None:
            before = datetime.now()

        query = {"timestamp": {"$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        if game_type is not None:
            query["game_type"] = game_type

        cursor = self.score_history.find(query).sort("timestamp", pymongo.ASCENDING)

        async for record in cursor:
            yield record

    async def get_game_score(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, before: datetime = None) -> int:
        # TODO: cache the results
        if before is None:
            before = datetime.now()

        query = {"timestamp": {"$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        if game_type is not None:
            query["game_type"] = game_type

        cursor = await self.score_history.aggregate([
            {"$match": query},
            {"$group": {
                "_id": None,
                "total_score": {"$sum": "$score"},
            }},
        ])
        result = await cursor.to_list(length=1)

        return result[0]["total_score"] if result else 0


async def test_attack_station_score_history():
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    eps = 0.1

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017"), time_base)
    await gl.clear_database()

    # Simulate
    scores = [100, 200, 300, -50, -100, -200, -300, -400, -500, -600, 400, 400, 400, 400, 400, 400, 400]
    player_id = 1
    station_id = 1
    for i, score in enumerate(scores):
        await gl.attack_station(player_id, station_id, score, time_base + timedelta(seconds=i + 0.5))

    # Test
    total_score = 0
    for i in range(len(scores)):
        # decay the score
        if total_score > 0:
            total_score = max(0, total_score - const.STATION_SCORE_DECAY_AMOUNT)
        elif total_score < 0:
            total_score = min(0, total_score + const.STATION_SCORE_DECAY_AMOUNT)

        assert total_score == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + eps))

        # add the score
        total_score = clamp(total_score + scores[i], const.STATION_SCORE_LB, const.STATION_SCORE_UB)

        assert total_score == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + 0.5 + eps))

    for i in range(len(scores), len(scores) + 10):
        # decay the score
        if total_score > 0:
            total_score = max(0, total_score - const.STATION_SCORE_DECAY_AMOUNT)
        elif total_score < 0:
            total_score = min(0, total_score + const.STATION_SCORE_DECAY_AMOUNT)

        assert total_score == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + eps))


async def test_game_score_history():
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    eps = 0.1

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017"), time_base)
    await gl.clear_database()

    table = {
        GameType.SHAKE_BADGE: [10, 15],
        GameType.DINO: [20, 25],
        GameType.SNAKE: [30, 35],
        GameType.TETRIS: [40, 45],
        GameType.CONNECT_SPONSOR: [50, 55],
        GameType.RECTF: [60, 65],
    }
    player_id = 1
    station_id = 1

    # Simulate
    for game_type, scores in table.items():
        await gl.receive_game_score(player_id, station_id, scores[0], game_type, time_base + timedelta(seconds=0.5))
        await gl.receive_game_score(player_id, station_id, scores[1], game_type, time_base + timedelta(seconds=1.5))

    # Test
    for game_type, scores in table.items():
        assert 0 == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=3 + eps))

    assert await gl.get_game_score(player_id=player_id, before=time_base + timedelta(seconds=0 + eps)) == 0
    assert await gl.get_game_score(player_id=player_id, before=time_base + timedelta(seconds=1 + eps)) == sum([
        scores[0] for _, scores in table.items()
    ])
    assert await gl.get_game_score(player_id=player_id, before=time_base + timedelta(seconds=2 + eps)) == sum([
        scores[0] + scores[1] for _, scores in table.items()
    ])
    assert await gl.get_game_score(player_id=player_id, before=time_base + timedelta(seconds=3 + eps)) == sum([
        scores[0] + scores[1] for _, scores in table.items()
    ])


if __name__ == "__main__":
    asyncio.run(test_attack_station_score_history())
    asyncio.run(test_game_score_history())
