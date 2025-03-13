from fastapi import FastAPI
from packet_processor import PacketProcessor
from crypto_auth_layer import CryptoAuthLayer
from game_logic import GameLogic
from config import Config
from schemas import Station, IrPacket, Display

config = Config("config.yaml")
app = FastAPI()
game_logic_instance = GameLogic(config=config)
crypto_auth_layer_instance = CryptoAuthLayer(config=config)
packet_processor_instance = PacketProcessor(
    config=config, crypto_auth=crypto_auth_layer_instance
)

router = APIRouter(prefix="/v1")

@app.get("/")
async def read_root():
    return {"message": "Hello World"}


@router.get("/tx")
async def tx() -> list[IrPacket]:
    # Backend asks the base station to send a packet.
    pass


@router.post("/rx")
async def rx(ir_packet: IrPacket):
    # Base station received a packet, sending it to the backend.
    pass


@router.get("/station-display")
async def station_display() -> Display:
    pass


app.include_router(router)