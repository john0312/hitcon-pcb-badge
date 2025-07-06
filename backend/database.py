from pymongo import AsyncMongoClient
import redis.asyncio as redis
from config import Config

config = Config("config.yaml")

mongo = AsyncMongoClient(f"mongodb://{config['mongo']['username']}:{config['mongo']['password']}@{config['mongo']['host']}:{config['mongo']['port']}?uuidRepresentation=standard")
db = mongo[config["mongo"]["db"]]

redis_client = redis.Redis(
    host=config["redis"]["host"],
    port=config["redis"]["port"]
)