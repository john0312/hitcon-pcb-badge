import uuid
import time
import asyncio
from dataclasses import dataclass, field, fields, MISSING
from datetime import datetime, timedelta
import pymongo
from enum import Enum
from redis.asyncio import Redis
import random
import itertools

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
    TAMA = "tama"
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
    PLAYER_BUFF_COLLECTION: str = "player_buff"

    STATION_COUNT: int = 20
    STATION_SCORE_LB: int = -1000
    STATION_SCORE_UB: int = 1000
    STATION_NEUTRAL_LB: int = -300
    STATION_NEUTRAL_UB: int = 300

    STATION_SCORE_DECAY_INTERVAL: int = 30 # seconds
    STATION_SCORE_DECAY_AMOUNT: int = 10

    STATION_SCORE_CACHE_MIN_INTERVAL: int = 30 # seconds

    GAME_SCORE_GRANULARITY: int | None = 10  # seconds

    BUFF_A_MODIFIER: float = 0.04
    BUFF_B_MODIFIER: float = 0.04

    SPONSOR_STATION_ID_LIST: list[int] = field(default_factory=lambda: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    SPONSOR_CONNECT_SCORE: int = 50
    SPONSOR_ALL_COLLECTED_BONUS: int = 300

    def reset(self):
        for field in fields(self):
            if field.default_factory is not MISSING:
                setattr(self, field.name, field.default_factory())
            else:
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
        self.player_buff = self.db[const.PLAYER_BUFF_COLLECTION]
        self.redis_client = redis_client
        # TODO: test if create index is necessary for performance

        if start_time is None:
            start_time = datetime.now()
        self.start_time = start_time

    async def clear_database(self):
        await self.attack_history.delete_many({})
        await self.score_history.delete_many({})
        await self.player_buff.delete_many({})

    def _round_to_granularity(self, timestamp: datetime):
        """
        Round the timestamp to the nearest granularity.
        """
        if const.GAME_SCORE_GRANULARITY is not None:
            seconds_from_start1 = round((timestamp - self.start_time).total_seconds())
            seconds_from_start2 = int(seconds_from_start1 // const.GAME_SCORE_GRANULARITY) * const.GAME_SCORE_GRANULARITY
            return self.start_time + timedelta(seconds=seconds_from_start2)
        return timestamp

    async def attack_station(self, player_id: int, station_id: int, amount: int, timestamp: datetime):
        """
        This method is called when a player attacks a station.
        This will add a record in the database. The station's score will be calculated
        based on the history of attacks.
        """
        # TODO: validate the player_id and the amount

        # Retrieve the buff
        async def _tmp():
            # cache
            if self.redis_client is not None:
                buff = await self.redis_client.get(f"player_buff:{player_id}")
                if buff is not None:
                    a, b = buff.decode().split(",")
                    return int(a), int(b)

            # database
            buff = await self.player_buff.find_one({"player_id": player_id, "latest": True})
            if buff is not None:
                a, b = buff.get("buff_a_count", 0), buff.get("buff_b_count", 0)
                if self.redis_client is not None:
                    await self.redis_client.set(f"player_buff:{player_id}", f"{a},{b}")
                return int(a), int(b)

            if self.redis_client is not None:
                await self.redis_client.set(f"player_buff:{player_id}", "0,0")
            return 0, 0

        buff_a_count, buff_b_count = await _tmp()

        # apply the buff
        amount_after_buff = int(amount * (1 + const.BUFF_A_MODIFIER * buff_a_count + const.BUFF_B_MODIFIER * buff_b_count))

        await self.attack_history.insert_one({
            "player_id": player_id,
            "station_id": station_id,
            "amount": amount_after_buff,
            "timestamp": timestamp,
            "amount_before_buff": amount,
            "buff_a_count": buff_a_count,
            "buff_b_count": buff_b_count,
            "buff_a_modifier": const.BUFF_A_MODIFIER,
            "buff_b_modifier": const.BUFF_B_MODIFIER,
        })

    async def get_station_attack_history(self, *, player_id: int = None, station_id: int = None, start: datetime = None, before: datetime = None):
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

    async def get_station_score(self, *, station_id: int = None, before: datetime = None) -> int:
        if before is None:
            before = datetime.now()

        # check cache
        cached_score = None
        last_cached_time = None
        if self.redis_client is not None:
            # redis's sorted set
            # key: f"station_score:{player_id}:{station_id}"
            # score: timestamp
            # member: f"{timestamp}:{score}" (note that sorted set requires member to be unique)

            # get the latest cached score before the given time
            tmp = await self.redis_client.zrange(
                f"station_score:{station_id}",
                start=before.timestamp(),
                end=self.start_time.timestamp(),
                desc=True,
                withscores=True,
                byscore=True,
                offset=0,
                num=1,
            )
            if tmp:
                cached_score = int(tmp[0][0].decode().rpartition(":")[2])
                last_cached_time = datetime.fromtimestamp(tmp[0][1])
                # print(f'cache hit: {cached_score} at {last_cached_time.isoformat()}')

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

        async for record in self.get_station_attack_history(station_id=station_id, start=start_time, before=before):
            proceed(record["timestamp"])
            total_score = clamp(total_score + record["amount"], const.STATION_SCORE_LB, const.STATION_SCORE_UB)

        proceed(before)

        if self.redis_client is not None:
            # Cache the total score, if it has been a while since the last cache
            if last_cached_time is None or (before - last_cached_time).total_seconds() >= const.STATION_SCORE_CACHE_MIN_INTERVAL:
                await self.redis_client.zadd(
                    f"station_score:{station_id}",
                    {f"{before.isoformat()}:{total_score}": before.timestamp()},
                )

        return total_score

    async def receive_game_score_single_player(self, player_id: int, station_id: int, score: int, game_type: GameType, timestamp: datetime, log_only: bool = False, sponsor_id: int = None):
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
                if sponsor_id is None:
                    raise ValueError("sponsor_id must be provided for CONNECT_SPONSOR game type")
                if sponsor_id not in const.SPONSOR_STATION_ID_LIST:
                    raise ValueError(f"sponsor_id {sponsor_id} is not a valid sponsor ID")

                # Get the list of collected sponsor IDs
                # TODO: cache? (profile the performance first)
                # TODO: There may be race condition (TOCTTOU), but maybe it is not a big deal
                sponsors = await self.score_history.distinct("sponsor_id", {"player_id": player_id, "game_type": GameType.CONNECT_SPONSOR})

                # Check if the player has already collected this sponsor badge
                if sponsor_id in sponsors:
                    return  # No need to insert a duplicate record

                # If the player has collected all sponsor badges, attack the station with a bonus score
                if len(sponsors) == len(const.SPONSOR_STATION_ID_LIST) - 1:
                    await self.attack_station(player_id, station_id, const.SPONSOR_ALL_COLLECTED_BONUS, timestamp)

        await self.score_history.insert_one({
            "player_id": player_id,
            "station_id": station_id,
            "sponsor_id": sponsor_id,
            "score": score,
            "game_type": game_type,
            "timestamp": timestamp,
            "log_only": log_only,
        })

    async def receive_game_score_two_player(self, two_player_event_id: uuid.UUID, player1_id: int, player2_id: int, station_id: int, score1: int, score2: int, game_type: GameType, timestamp: datetime, log_only: bool = False):
        match game_type:
            case GameType.SNAKE:
                # winner has double score
                if score1 > score2:
                    score1 *= 2
                elif score1 < score2:
                    score2 *= 2

            case GameType.TETRIS:
                # winner has double score
                if score1 > score2:
                    score1 *= 2
                elif score1 < score2:
                    score2 *= 2

            case GameType.TAMA:
                # TODO: validate the score and timestamp
                log_only = True

        await self.score_history.insert_many([
            {
                "player_id": player1_id,
                "station_id": station_id,
                "score": score1,
                "game_type": game_type,
                "timestamp": timestamp,
                "two_player_event_id": two_player_event_id,
                "log_only": log_only,
            },
            {
                "player_id": player2_id,
                "station_id": station_id,
                "score": score2,
                "game_type": game_type,
                "timestamp": timestamp,
                "two_player_event_id": two_player_event_id,
                "log_only": log_only,
            },
        ])

    async def get_game_history(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, num_of_player: GameNumOfPlayerType = None, start: datetime = None, before: datetime = None, log_only: bool = None):
        # TODO: support "log_only" field to filter out log-only events
        if start is None:
            start = self.start_time
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

        if log_only is not None:
            query["log_only"] = log_only

        if num_of_player == GameNumOfPlayerType.SINGLE:
            query["two_player_event_id"] = {"$exists": False}
        elif num_of_player == GameNumOfPlayerType.TWO:
            query["two_player_event_id"] = {"$exists": True}

        cursor = self.score_history.find(query).sort("timestamp", pymongo.ASCENDING)

        async for record in cursor:
            yield record

    async def get_game_score(self, *, player_id: int = None, station_id: int = None, game_type: GameType = None, num_of_player: GameNumOfPlayerType = None, before: datetime = None) -> int:
        # Note that only log_only=False records are considered for the score calculation

        if before is None:
            before = datetime.now()
        before = self._round_to_granularity(before)

        if num_of_player is None:
            num_of_player = GameNumOfPlayerType.ALL

        query = {"timestamp": {"$lt": before}, "log_only": False}

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

    async def get_player_scoreboard(self, before: datetime = None):
        """
        Get all players' scores in the game.
        Return like [
            {
                "player_id": 101,
                "scores": {
                    "shake_badge": 100,
                    "dino": 200,
                    ...
                },
                "total_score": 2100,
                "connected_sponsors": [1, 2, 3],
            },
            ...
        ]
        """

        if before is None:
            before = datetime.now()
        before = self._round_to_granularity(before)

        # check cache, if granularity is set
        # if granularity is not set, the cache will be meaningless, since it is practically impossible to hit the cache with two same "before" timestamps
        if self.redis_client is not None and const.GAME_SCORE_GRANULARITY is not None:
            # TODO: cache
            pass

        cursor = await self.score_history.aggregate([
            {
                "$match": {
                    "log_only": { "$ne": True },
                    "timestamp": { "$lt": before }
                }
            },
            {
                "$group": {
                    "_id": {
                        "player_id": "$player_id",
                        "game_type": "$game_type"
                    },
                    "total_score": { "$sum": "$score" },
                    "connected_sponsors": { "$addToSet": "$sponsor_id" }
                }
            },
            {
                "$group": {
                    "_id": "$_id.player_id",
                    "scores": {
                        "$push": { "k": "$_id.game_type", "v": "$total_score" }
                    },
                    "total_score": { "$sum": "$total_score" },
                    "connected_sponsors": { "$addToSet": "$connected_sponsors" }
                }
            },
            {
                "$project": {
                    "_id": 0,
                    "player_id": "$_id",
                    "scores": { "$arrayToObject": "$scores" },
                    "total_score": 1,
                    "connected_sponsors": {
                        "$filter": {
                            "input": {
                                "$reduce": {
                                    "input": "$connected_sponsors",
                                    "initialValue": [],
                                    "in": { "$setUnion": ["$$value", "$$this"] }
                                }
                            },
                            "as": "item",
                            "cond": { "$ne": ["$$item", None] }
                        }
                    }
                }
            }
        ])
        result = await cursor.to_list(length=None)

        # give missing game types a score of 0
        for record in result:
            for game_type in GameType:
                if game_type not in record["scores"]:
                    record["scores"][game_type] = 0

        # If granularity is set, cache the score
        if self.redis_client is not None and const.GAME_SCORE_GRANULARITY is not None:
            # TODO: cache
            pass

        return result

    async def update_player_buff(self, player_id: int, buff_a_count: int, buff_b_count: int, timestamp: datetime):
        """
        **Assume that `timestamp` is MONOTONIC (i.e., it is always later than the previous timestamp).**

        Update the player's buff (possibly a result of solving CTF challenges).
        buff_a and buff_b has different parameter on the modifier.
        The attack power = amount * modifier.
        """

        # Check cache
        if self.redis_client is not None:
            await self.redis_client.set(f"player_buff:{player_id}", f"{buff_a_count},{buff_b_count}")

        # Update the player's buff in the database
        await self.player_buff.update_one(
            {"player_id": player_id, "latest": True},
            {
                "$set": {
                    "buff_a_count": buff_a_count,
                    "buff_b_count": buff_b_count,
                    "timestamp": timestamp,
                },
                "$setOnInsert": {
                    "player_id": player_id,
                    "latest": True,
                },
            },
            upsert=True,
        )

        # Save for history
        await self.player_buff.insert_one({
            "player_id": player_id,
            "buff_a_count": buff_a_count,
            "buff_b_count": buff_b_count,
            "timestamp": timestamp,
            "latest": False,  # for retaining the history
        })


async def profile_attack_station(with_redis, time_limit_sec):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
    await gl.clear_database()

    # 2 days, 8 hours per day, 21 station
    # Assume 800 players, each player attack station every 30 second

    random.seed("chiffoncake") # for reproducibility
    all_seconds = 2 * 8 * 3600
    num_players = 800
    num_stations = 21
    attack_interval = 30
    total_attacks = num_players * (all_seconds // attack_interval)

    print(f"Profiling attack_station {with_redis=} {time_limit_sec=}")

    # prepare data
    start = time.perf_counter()
    count = 0
    # for player_id, t in tqdm(itertools.product(range(1, num_players + 1), range(0, all_seconds, attack_interval)), desc="Simulating attacks", total=total_attacks):
    for player_id, t in itertools.product(range(1, num_players + 1), range(0, all_seconds, attack_interval)):
        if time.perf_counter() - start > time_limit_sec:
            break
        station_id = random.choice(range(1, num_stations + 1))
        sign_ = 1 if player_id <= num_players // 2 else -1
        await gl.attack_station(player_id, station_id, sign_ * random.randint(0, 100), time_base + timedelta(seconds=t))
        count += 1

    print(f"Executed {count}/{total_attacks} attacks within {time_limit_sec} seconds. (avg. {count / time_limit_sec:.2f} attacks/sec)")


_executed_test = set()
def test_func(func):
    # use this decorator to make sure all tests are executed
    async def wrapper(*args, **kwargs):
        _executed_test.add(func.__name__)
        return await func(*args, **kwargs)
    return wrapper


@test_func
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


@test_func
async def test_attack_station_score_buff(buff_a_count, buff_b_count, with_redis = False):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
    await gl.clear_database()

    player_id = 1
    station_id = 1

    await gl.update_player_buff(player_id, buff_a_count=buff_a_count, buff_b_count=buff_b_count, timestamp=time_base)

    # Simulate
    scores = [100, 200, 300, -50, -100, -200, -300, -400, -500, -600, 400, 400, 400, 400, 400, 400, 400]
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
        modifier = 1 + const.BUFF_A_MODIFIER * buff_a_count + const.BUFF_B_MODIFIER * buff_b_count
        total_score = clamp(total_score + int(scores[i] * modifier), const.STATION_SCORE_LB, const.STATION_SCORE_UB)

        assert total_score == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=i + 0.5 + eps))


@test_func
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
        await gl.receive_game_score_single_player(player_id, station_id, scores[0], game_type, time_base + timedelta(seconds=0.5), sponsor_id=(None if game_type != GameType.CONNECT_SPONSOR else const.SPONSOR_STATION_ID_LIST[0]))
        await gl.receive_game_score_single_player(player_id, station_id, scores[1], game_type, time_base + timedelta(seconds=1.5), sponsor_id=(None if game_type != GameType.CONNECT_SPONSOR else const.SPONSOR_STATION_ID_LIST[1]))

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


@test_func
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
        GameType.SNAKE: [[15, 20], [30, 25]],
        GameType.TETRIS: [[25, 30], [40, 35]],
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
    # Note that winner's score is doubled in SNAKE and TETRIS
    for game_type, scores in table.items():
        assert 0 == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=0 + eps))
        assert scores[0][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=1 + eps))
        assert scores[0][0] + 2*scores[1][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=2 + eps))
        assert scores[0][0] + 2*scores[1][0] == await gl.get_game_score(player_id=player1_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=3 + eps))

        assert 0 == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=0 + eps))
        assert 2*scores[0][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=1 + eps))
        assert 2*scores[0][1] + scores[1][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=2 + eps))
        assert 2*scores[0][1] + scores[1][1] == await gl.get_game_score(player_id=player2_id, station_id=station_id, game_type=game_type, num_of_player=GameNumOfPlayerType.TWO, before=time_base + timedelta(seconds=3 + eps))


@test_func
async def test_game_score_log_only():
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    eps = 0.1

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), None, time_base)
    await gl.clear_database()

    player1_id = 1
    player2_id = 2
    station_id = 1

    # Simulate
    two_player_event_id = uuid.uuid4()
    await gl.receive_game_score_two_player(two_player_event_id, player1_id, player2_id, station_id, 10, -10, GameType.TAMA, time_base + timedelta(seconds=0.5))
    await gl.receive_game_score_two_player(two_player_event_id, player1_id, player2_id, station_id, 20, -20, GameType.TAMA, time_base + timedelta(seconds=1.5))

    # Test
    assert 0 == await gl.get_game_score(player_id=player1_id, station_id=station_id, before=time_base + timedelta(seconds=0 + eps))
    assert 0 == await gl.get_game_score(player_id=player1_id, station_id=station_id, before=time_base + timedelta(seconds=1 + eps))
    assert 0 == await gl.get_game_score(player_id=player1_id, station_id=station_id, before=time_base + timedelta(seconds=2 + eps))
    assert 0 == await gl.get_game_score(player_id=player2_id, station_id=station_id, before=time_base + timedelta(seconds=0 + eps))
    assert 0 == await gl.get_game_score(player_id=player2_id, station_id=station_id, before=time_base + timedelta(seconds=1 + eps))
    assert 0 == await gl.get_game_score(player_id=player2_id, station_id=station_id, before=time_base + timedelta(seconds=2 + eps))


@test_func
async def test_sponsor_bonus(with_redis = False):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1000 # a very large value to avoid decay during the test
    const.SPONSOR_STATION_ID_LIST = [1, 2, 3]
    const.GAME_SCORE_GRANULARITY = 0.1
    eps = 0.1

    if with_redis:
        redis_client = Redis(host='localhost', port=6379)
        await redis_client.flushall()  # Clear all keys in Redis for testing
    else:
        redis_client = None

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), redis_client, time_base)
    await gl.clear_database()

    player_id = 1
    station_id = 1
    player_score = 100

    # Score should be 0 when sponsor are not fully collected
    for i in range(len(const.SPONSOR_STATION_ID_LIST) - 1):
        await gl.receive_game_score_single_player(player_id, station_id, player_score, GameType.CONNECT_SPONSOR, time_base + timedelta(seconds=2*i + 0.5), sponsor_id=const.SPONSOR_STATION_ID_LIST[i])
        assert 0 == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=2*i + 0.5 + eps))

        # sponsor score should not duplicate
        assert (i+1) * player_score == await gl.get_game_score(player_id=player_id, game_type=GameType.CONNECT_SPONSOR, before=time_base + timedelta(seconds=2*i + 0.5 + eps))
        await gl.receive_game_score_single_player(player_id, station_id, player_score, GameType.CONNECT_SPONSOR, time_base + timedelta(seconds=2*i + 1 + 0.5), sponsor_id=const.SPONSOR_STATION_ID_LIST[i])
        assert (i+1) * player_score == await gl.get_game_score(player_id=player_id, game_type=GameType.CONNECT_SPONSOR, before=time_base + timedelta(seconds=2*i + 1 + 0.5 + eps))

    # Buff should be applied when all sponsors are collected
    await gl.receive_game_score_single_player(player_id, station_id, player_score, GameType.CONNECT_SPONSOR, time_base + timedelta(seconds=len(const.SPONSOR_STATION_ID_LIST) - 1), sponsor_id=const.SPONSOR_STATION_ID_LIST[-1])
    assert const.SPONSOR_ALL_COLLECTED_BONUS == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=len(const.SPONSOR_STATION_ID_LIST) - 1 + eps))

    # Duplicate sponsor score should have no effect
    for i in range(len(const.SPONSOR_STATION_ID_LIST)):
        await gl.receive_game_score_single_player(player_id, station_id, player_score, GameType.CONNECT_SPONSOR, time_base + timedelta(seconds=len(const.SPONSOR_STATION_ID_LIST) + i + 0.5), sponsor_id=const.SPONSOR_STATION_ID_LIST[-1])
        assert const.SPONSOR_ALL_COLLECTED_BONUS == await gl.get_station_score(station_id=station_id, before=time_base + timedelta(seconds=len(const.SPONSOR_STATION_ID_LIST) + i + 0.5 + eps))


@test_func
async def test_scoreboard_api(game_score_granularity = None):
    const.reset()
    const.STATION_SCORE_DECAY_INTERVAL = 1
    const.GAME_SCORE_GRANULARITY = game_score_granularity
    eps = 0.1

    time_base = datetime.now()
    gl = _GameLogic(pymongo.AsyncMongoClient("mongodb://localhost:27017?uuidRepresentation=standard"), None, time_base)
    await gl.clear_database()

    operations = [
        # num of player, game type, score, sponsor_id, timestamp
        (GameNumOfPlayerType.SINGLE, GameType.SHAKE_BADGE, 4, None, time_base + timedelta(seconds=0.5)),
        (GameNumOfPlayerType.SINGLE, GameType.DINO, 5, None, time_base + timedelta(seconds=1.5)),
        (GameNumOfPlayerType.SINGLE, GameType.CONNECT_SPONSOR, 6, 1, time_base + timedelta(seconds=2.5)),
        (GameNumOfPlayerType.SINGLE, GameType.CONNECT_SPONSOR, 7, 2, time_base + timedelta(seconds=2.5)),
        (GameNumOfPlayerType.SINGLE, GameType.CONNECT_SPONSOR, 8, 4, time_base + timedelta(seconds=2.5)),

        # num of player, game type, player 1 score, player 2 score, timestamp
        (GameNumOfPlayerType.TWO, GameType.SNAKE, 3, 4, time_base + timedelta(seconds=4.5)),
        (GameNumOfPlayerType.TWO, GameType.TETRIS, 6, 5, time_base + timedelta(seconds=4.5)),
        (GameNumOfPlayerType.TWO, GameType.TAMA, 7, 8, time_base + timedelta(seconds=5.5)), # log only
    ]
    total_score_1 = 4 + 5 + 6 + 7 + 8 + 3 + 6*2 # ignore TAMA's score
    total_score_2 = 4*2 + 5 # ignore TAMA's score
    sponsors_1 = [1, 2, 4]
    sponsors_2 = []

    for args in operations:
        if args[0] == GameNumOfPlayerType.SINGLE:
            _, game_type, score, sponsor_id, timestamp = args
            await gl.receive_game_score_single_player(
                player_id=1,
                station_id=1,
                score=score,
                game_type=game_type,
                timestamp=timestamp,
                sponsor_id=sponsor_id,
            )
        elif args[0] == GameNumOfPlayerType.TWO:
            _, game_type, score1, score2, timestamp = args
            two_player_event_id = uuid.uuid4()
            await gl.receive_game_score_two_player(
                two_player_event_id=two_player_event_id,
                player1_id=1,
                player2_id=2,
                station_id=1,
                score1=score1,
                score2=score2,
                game_type=game_type,
                timestamp=timestamp,
            )

    # Test
    scoreboard = await gl.get_player_scoreboard(before=time_base + timedelta(seconds=10 + eps))
    scoreboard.sort(key=lambda x: x["player_id"])  # Sort by player_id for consistency

    assert scoreboard[0]["player_id"] == 1
    assert scoreboard[0]["total_score"] == total_score_1
    assert set(scoreboard[0]["connected_sponsors"]) == set(sponsors_1)
    assert scoreboard[0]["scores"] == {
        GameType.SHAKE_BADGE: 4,
        GameType.DINO: 5,
        GameType.SNAKE: 3,
        GameType.TETRIS: 6*2,
        GameType.TAMA: 0,
        GameType.CONNECT_SPONSOR: 6 + 7 + 8,
        GameType.RECTF: 0,
    }
    assert scoreboard[1]["player_id"] == 2
    assert scoreboard[1]["total_score"] == total_score_2
    assert set(scoreboard[1]["connected_sponsors"]) == set(sponsors_2)
    assert scoreboard[1]["scores"] == {
        GameType.SHAKE_BADGE: 0,
        GameType.DINO: 0,
        GameType.SNAKE: 4*2,
        GameType.TETRIS: 5,
        GameType.TAMA: 0,
        GameType.CONNECT_SPONSOR: 0,
        GameType.RECTF: 0,
    }


if __name__ == "__main__":
    asyncio.run(profile_attack_station(with_redis=False, time_limit_sec=5))
    asyncio.run(profile_attack_station(with_redis=True, time_limit_sec=5))
    # exit(0)  # Exit early for profiling

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
    asyncio.run(test_attack_station_score_buff(buff_a_count=2, buff_b_count=3))
    asyncio.run(test_attack_station_score_buff(buff_a_count=0, buff_b_count=0))
    asyncio.run(test_attack_station_score_buff(buff_a_count=10, buff_b_count=5))
    asyncio.run(test_attack_station_score_buff(buff_a_count=2, buff_b_count=3, with_redis=True))
    asyncio.run(test_attack_station_score_buff(buff_a_count=0, buff_b_count=0, with_redis=True))
    asyncio.run(test_attack_station_score_buff(buff_a_count=10, buff_b_count=5, with_redis=True))
    asyncio.run(test_game_score_log_only())
    asyncio.run(test_sponsor_bonus())
    asyncio.run(test_sponsor_bonus(with_redis=True))
    asyncio.run(test_scoreboard_api(game_score_granularity=0.1))
    asyncio.run(test_scoreboard_api(game_score_granularity=0.3))

    import sys
    import inspect
    current_module = sys.modules[__name__]
    all_tests = {
        name
        for name, _ in inspect.getmembers(current_module, inspect.iscoroutinefunction)
        if name.startswith("test_")
    }

    assert _executed_test == all_tests, f"Not all tests were executed: {all_tests - _executed_test}"
    print("All tests passed!")
