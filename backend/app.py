from fastapi import FastAPI, APIRouter, Depends, Security, HTTPException
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from packet_processor import PacketProcessor
from crypto_auth_layer import CryptoAuthLayer
from game_logic import GameLogic
from config import Config
from schemas import Station, IrPacketRequestSchema, Display

config = Config("config.yaml")
app = FastAPI()
game_logic_instance = GameLogic(config=config)
crypto_auth_layer_instance = CryptoAuthLayer(config=config)
packet_processor_instance = PacketProcessor(
    config=config, crypto_auth=crypto_auth_layer_instance
)

router = APIRouter(prefix="/v1")
security = HTTPBearer()

async def get_station(credentials: HTTPAuthorizationCredentials = Security(security)) -> Station:
    key = credentials.credentials

    # TODO: check key against database
    # if key not in stations:
    #     raise HTTPException(status_code=401, detail="Invalid key")

    return Station(station_id="1", station_key="key", display=Display(bar_1="bar1", bar_2="bar2", winning_color="red"), tx=[], rx=[])


@app.get("/")
async def read_root():
    return {"message": "Hello World"}

#fake scores
@app.get("/api/score", response_model=List[ScoreEntry])
async def get_fake_scores():
    return [
        {"name": "tony", "uid": 101, "score": 9527},
        {"name": "chen", "uid": 102, "score": 5566},
        {"name": "sherry", "uid": 103, "score": 857}
    ]

@router.get("/tx")
async def tx(station: Station = Depends(get_station)) -> list[IrPacketRequestSchema]:
    # Backend asks the base station to send a packet.
    pass


@router.post("/rx")
async def rx(ir_packet: IrPacketRequestSchema, station: Station = Depends(get_station)):
    # Base station received a packet, sending it to the backend.
    pass


@router.get("/station-display")
async def station_display(station: Station = Depends(get_station)) -> Display:
    pass


app.include_router(router)
