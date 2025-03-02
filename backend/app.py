from fastapi import FastAPI
from packet_processor import PacketProcessor
from crypto_auth_layer import CryptoAuthLayer
from game_logic import GameLogic
from config import Config

config = Config("config.yaml")
app = FastAPI()
game_logic_instance = GameLogic(config=config)
crypto_auth_layer_instance = CryptoAuthLayer(config=config)
packet_processor_instance = PacketProcessor(
    config=config, crypto_auth=crypto_auth_layer_instance
)


@app.get("/")
async def read_root():
    return {"message": "Hello World"}
