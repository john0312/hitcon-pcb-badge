from fastapi import FastAPI, APIRouter, Depends, Security, HTTPException
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from pymongo import AsyncMongoClient
from packet_processor import PacketProcessor
from crypto_auth_layer import CryptoAuthLayer
from game_logic import GameLogic
from config import Config
from schemas import Station, IrPacketRequestSchema, Display

config = Config("config.yaml")
mongo = AsyncMongoClient(f"mongodb://{config['mongo']['username']}:{config['mongo']['password']}@{config['mongo']['host']}:{config['mongo']['port']}")
db = mongo[config["mongo"]["db"]]
stations = db["stations"]

app = FastAPI()
game_logic_instance = GameLogic(config=config)
crypto_auth_layer_instance = CryptoAuthLayer(config=config)
packet_processor_instance = PacketProcessor(
    config=config, crypto_auth=crypto_auth_layer_instance, db=db
)

router = APIRouter(prefix="/v1")
security = HTTPBearer()

async def get_station(credentials: HTTPAuthorizationCredentials = Security(security)) -> Station:
    key = credentials.credentials

    station = await stations.find_one({"station_key": key})
    if station is None:
        raise HTTPException(status_code=403, detail="Invalid station key")

    return Station(**station)


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
async def tx(background_tasks: BackgroundTasks, station: Station = Depends(get_station)) -> list[IrPacketRequestSchema]:
    # Backend asks the base station to send a packet.
    packets = packet_processor_instance.has_packet_for_tx(station)

    ret = []
    async for packet in packets:
        ret.append(packet)

    return ret


@router.post("/rx")
async def rx(ir_packet: IrPacketRequestSchema, station: Station = Depends(get_station)):
    # Base station received a packet, sending it to the backend.
    try:
        await packet_processor_instance.on_receive_packet(ir_packet, station)
    except Exception as e:
        print(f"Error processing packet: {e}")
        raise HTTPException(status_code=500, detail=str(e))

    return {"status": "ok"}


@router.get("/station-display")
async def station_display(station: Station = Depends(get_station)) -> Display:
    pass


app.include_router(router)
