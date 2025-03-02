from fastapi import FastAPI
from packet_processor import PacketProcessor
from crypto_auth_layer import CryptoAuthLayer
from game_logic import GameLogic

app = FastAPI()
game_logic_instance = GameLogic()
crypto_auth_layer_instance = CryptoAuthLayer()
packet_processor_instance = PacketProcessor(crypto_auth=crypto_auth_layer_instance)


@app.get("/")
async def read_root():
    return {"message": "Hello World"}
