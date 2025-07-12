import uuid
import asyncio
from dataclasses import dataclass, fields
from datetime import datetime, timedelta
import pymongo
from enum import Enum
from redis.asyncio import Redis
import random

try:
    from enum import StrEnum
except ImportError:
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


class GameNumOfPlayerType(Enum):
    ALL = "all"
    SINGLE = "single"
    TWO = "two"


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

    STATION_SCORE_CACHE_MIN_INTERVAL: int = 10 # seconds

    GAME_SCORE_GRANULARITY: int | None = 10  # seconds

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
    def __init__(self, mongo_client: pymongo.AsyncMongoClient, redis_client: Redis = None, start_time: datetime = None):
        # TODO: maybe use config to update constants
        self.db = mongo_client[const.DATABASE_NAME]
        self.attack_history = self.db[const.ATTACK_HISTORY_COLLECTION]
        self.score_history = self.db[const.SCORE_HISTORY_COLLECTION]
        self.redis_client = redis_client

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

    async def get_station_score_history(self, *, player_id: int = None, station_id: int = None, start: datetime = None, before: datetime = None):
        if start is None:
            start = self.start_time
        if before is None:
            before = datetime.now()

        query = {"timestamp": {"$gte": start, "$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        cursor = self.attack_history.find(query).sort("timestamp", pymongo.ASCENDING)

        async for record in cursor:
            yield record

    async def get_station_score(self, *, player_id: int = None, station_id: int = None, before: datetime = None) -> int:
        if before is None:
            before = datetime.now()

        # check cache
        cached_score = None
        if self.redis_client is not None:
            # check if we can directly use the latest cached score
            tmp = await self.redis_client.get(f"latest_station_score_time:{player_id}:{station_id}")
            if tmp is not None:
                last_cached_time = datetime.fromisoformat(tmp.decode())
                if last_cached_time < before:
                    tmp = await self.redis_client.get(f"station_score:{player_id}:{station_id}:{last_cached_time.isoformat()}")
                    if tmp is not None:
                        cached_score = int(tmp)

            else:
                # we are not getting the latest cached score (maybe calculating history score)
                # so we need to iterate through the cached scores to find the latest one before the given time
                # this is not efficient, but it is a fallback
                last_cached_time = self.start_time
                async for key in self.redis_client.scan_iter(match=f"station_score:{player_id}:{station_id}:*"):
                    # Extract the timestamp from the key and find the latest one
                    cached_time = datetime.fromisoformat(key.decode().split(":", maxsplit=3)[-1])
                    if cached_time < before and cached_time > last_cached_time:
                        last_cached_time = cached_time

                tmp = await self.redis_client.get(f"station_score:{player_id}:{station_id}:{last_cached_time.isoformat()}")
                if tmp is not None:
                    cached_score = int(tmp)

        if cached_score is not None:
            total_score = cached_score
            start_time = last_cached_time
        else:
            total_score = 0
            start_time = self.start_time
        time_pointer = self.start_time

        def proceed(until: datetime):
            nonlocal time_pointer
            nonlocal total_score
            if time_pointer >= until:
                return
            # Decay the score based on the time passed
            while time_pointer <= until:
                if time_pointer > start_time:
                    total_score += -1 * sign(total_score) * min(const.STATION_SCORE_DECAY_AMOUNT, abs(total_score))
                time_pointer += timedelta(seconds=const.STATION_SCORE_DECAY_INTERVAL)

        async for record in self.get_station_score_history(player_id=player_id, station_id=station_id, start=start_time, before=before):
            proceed(record["timestamp"])
            total_score = clamp(total_score + record["amount"], const.STATION_SCORE_LB, const.STATION_SCORE_UB)

        proceed(before)

        if self.redis_client is not None:
            # Cache the total score, if it has been a while since the last cache
            if (before - last_cached_time).total_seconds() >= const.STATION_SCORE_CACHE_MIN_INTERVAL:
                await self.redis_client.set(f"station_score:{player_id}:{station_id}:{before.isoformat()}", total_score)

                # Update the latest cached time if it is the newest one
                await self.redis_client.set(f"latest_station_score_time:{player_id}:{station_id}", before.isoformat(), nx=True)
                tmp = await self.redis_client.get(f"latest_station_score_time:{player_id}:{station_id}")
                if before > datetime.fromisoformat(tmp.decode()):
                    await self.redis_client.set(f"latest_station_score_time:{player_id}:{station_id}", before.isoformat())

        return total_score

    async def receive_game_score_single_player(self, player_id: int, station_id: int, score: int, game_type: GameType, timestamp: datetime):
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

    async def receive_game_score_two_player(self, two_player_event_id: uuid.UUID, player1_id: int, player2_id: int, station_id: int, score1: int, score2: int, game_type: GameType, timestamp: datetime):
        match game_type:
            case GameType.DINO:
                # TODO: validate the score and timestamp
                pass

            case GameType.SNAKE:
                # TODO: validate the score and timestamp
                pass

            case GameType.TETRIS:
                # TODO: validate the score and timestamp
                pass

        await self.score_history.insert_many([
            {
                "player_id": player1_id,
                "station_id": station_id,
                "score": score1,
                "game_type": game_type,
                "timestamp": timestamp,
                "two_player_event_id": two_player_event_id,
            },
            {
                "player_id": player2_id,
                "station_id": station_id,
                "score": score2,
                "game_type": game_type,
                "timestamp": timestamp,
                "two_player_event_id": two_player_event_id,
            },
        ])

    async def get_game_history(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, num_of_player: GameNumOfPlayerType = None, start: datetime = None, before: datetime = None):
        if before is None:
            before = datetime.now()

        if num_of_player is None:
            num_of_player = GameNumOfPlayerType.ALL

        query = {"timestamp": {"$gte": start, "$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        if game_type is not None:
            query["game_type"] = game_type

        if num_of_player == GameNumOfPlayerType.SINGLE:
            query["two_player_event_id"] = {"$exists": False}
        elif num_of_player == GameNumOfPlayerType.TWO:
            query["two_player_event_id"] = {"$exists": True}

        cursor = self.score_history.find(query).sort("timestamp", pymongo.ASCENDING)

        async for record in cursor:
            yield record

    async def get_game_score(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, num_of_player: GameNumOfPlayerType = None, before: datetime = None) -> int:
        if before is None:
            before = datetime.now()

        if num_of_player is None:
            num_of_player = GameNumOfPlayerType.ALL

        if const.GAME_SCORE_GRANULARITY is not None:
            # Round the before time to the nearest granularity
            seconds_from_start1 = round((before - self.start_time).total_seconds())
            seconds_from_start2 = int(seconds_from_start1 // const.GAME_SCORE_GRANULARITY) * const.GAME_SCORE_GRANULARITY
            before = self.start_time + timedelta(seconds=seconds_from_start2)

        query = {"timestamp": {"$lt": before}}

        if player_id is not None:
            query["player_id"] = player_id

        if station_id is not None:
            query["station_id"] = station_id

        if game_type is not None:
            query["game_type"] = game_type

        if num_of_player == GameNumOfPlayerType.SINGLE:
            query["two_player_event_id"] = {"$exists": False}
        elif num_of_player == GameNumOfPlayerType.TWO:
            query["two_player_event_id"] = {"$exists": True}

        # check cache, if granularity is set
        # if granularity is not set, the cache will be meaningless, since it is practically impossible to hit the cache with two same "before" timestamps
        if self.redis_client is not None and const.GAME_SCORE_GRANULARITY is not None:
            tmp = await self.redis_client.get(f"game_score:{player_id}:{station_id}:{game_type}:{num_of_player}:{before.isoformat()}")
            if tmp is not None:
                return int(tmp)

        cursor = await self.score_history.aggregate([
            {"$match": query},
            {"$group": {
                "_id": None,
                "total_score": {"$sum": "$score"},
            }},
        ])
        result = await cursor.to_list(length=1)

        score = result[0]["total_score"] if result else 0

        # If granularity is set, cache the score
        if self.redis_client is not None and const.GAME_SCORE_GRANULARITY is not None:
            # Cache the score, if it has been a while since the last cache
            await self.redis_client.set(f"game_score:{player_id}:{station_id}:{game_type}:{num_of_player}:{before.isoformat()}", score)

        return score


async def test_attack_station_score_history(with_redis = False, cache_min_interval = None):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    const.STATION_SCORE_CACHE_MIN_INTERVAL = 1 if cache_min_interval is None else cache_min_interval
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
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

    # test, random order query
    ground_truth = [
        (
            time_base + timedelta(seconds=i + eps),
            await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + eps)),
        )
        for i in range(len(scores))
    ] + [
        (
            time_base + timedelta(seconds=i + 0.5 + eps),
            await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + 0.5 + eps)),
        )
        for i in range(len(scores))
    ] + [
        (
            time_base + timedelta(seconds=i + eps),
            await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + eps)),
        )
        for i in range(len(scores), len(scores) + 10)
    ]
    random.seed(42)  # For reproducibility
    random.shuffle(ground_truth)
    if with_redis:
        # test if the cache works in random order
        await redis_client.flushall()
    for timestamp, expected_score in ground_truth:
        assert expected_score == await gl.get_station_score(station_id=station_id, before=timestamp)


async def test_game_score_history_single_player(with_redis = False, game_score_granularity = None):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    const.GAME_SCORE_GRANULARITY = game_score_granularity
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
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
        await gl.receive_game_score_single_player(player_id, station_id, scores[0], game_type, time_base + timedelta(seconds=0.5))
        await gl.receive_game_score_single_player(player_id, station_id, scores[1], game_type, time_base + timedelta(seconds=1.5))

    # Test
    for game_type, scores in table.items():
        assert 0 == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, before=time_base + timedelta(seconds=3 + eps))

        assert 0 == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.SINGLE, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.SINGLE, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.SINGLE, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0] + scores[1] == await gl.get_game_score(player_id=player_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.SINGLE, before=time_base + timedelta(seconds=3 + eps))

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


async def test_game_score_history_two_player(with_redis = False, game_score_granularity = None):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    const.GAME_SCORE_GRANULARITY = game_score_granularity
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
    await gl.clear_database()

    table = {
        GameType.DINO: [[10, -10], [-15, 15]],
        GameType.SNAKE: [[-20, 20], [25, -25]],
        GameType.TETRIS: [[30, -30], [35, -35]],
    }
    player1_id = 1
    player2_id = 2
    station_id = 1

    # Simulate
    for game_type, scores in table.items():
        two_player_event_id = uuid.uuid4()
        await gl.receive_game_score_two_player(two_player_event_id, player1_id, player2_id, station_id, scores[0][0], scores[0][1], game_type, time_base + timedelta(seconds=0.5))
        await gl.receive_game_score_two_player(two_player_event_id, player1_id, player2_id, station_id, scores[1][0], scores[1][1], game_type, time_base + timedelta(seconds=1.5))

    # Test
    for game_type, scores in table.items():
        assert 0 == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0][0] + scores[1][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0][0] + scores[1][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=3 + eps))

        assert 0 == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0][1] + scores[1][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0][1] + scores[1][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=3 + eps))


if __name__ == "__main__":
    asyncio.run(test_attack_station_score_history())
    asyncio.run(test_game_score_history_single_player())
    asyncio.run(test_game_score_history_two_player())
    asyncio.run(test_attack_station_score_history(with_redis=True, cache_min_interval=0.1))
    asyncio.run(test_attack_station_score_history(with_redis=True, cache_min_interval=0.4))
    asyncio.run(test_attack_station_score_history(with_redis=True, cache_min_interval=1.6))
    asyncio.run(test_attack_station_score_history(with_redis=True, cache_min_interval=6.4))
    asyncio.run(test_game_score_history_single_player(with_redis=True, game_score_granularity=0.1))
    asyncio.run(test_game_score_history_single_player(with_redis=True, game_score_granularity=0.3))
    asyncio.run(test_game_score_history_single_player(with_redis=False, game_score_granularity=0.1))
    asyncio.run(test_game_score_history_single_player(with_redis=False, game_score_granularity=0.3))
    asyncio.run(test_game_score_history_two_player(with_redis=True, game_score_granularity=0.1))
    asyncio.run(test_game_score_history_two_player(with_redis=True, game_score_granularity=0.3))
    asyncio.run(test_game_score_history_two_player(with_redis=False, game_score_granularity=0.1))
    asyncio.run(test_game_score_history_two_player(with_redis=False, game_score_granularity=0.3))
    print("All tests passed!")
