from fastapi import FastAPI, APIRouter, Depends, Security, HTTPException
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from packet_processor import PacketProcessor
from badge_link_controller import BadgeLinkController
from game_logic_controller import GameLogicController
from config import Config
from database import db
from schemas import Station, IrPacketRequestSchema, Display, ScoreEntry, ReCTFScoreSchema, BadgeLinkSchema

config = Config("config.yaml")

stations = db["stations"]

app = FastAPI()
packet_processor_instance = PacketProcessor(config=config)

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

## ====== Public API Endpoints ======
@app.get("/api/scores", response_model=list[ScoreEntry])
async def get_scoreboard():
    # TODO: return scoreboard from GameLogic
    return [
        {
            "name": "tony",
            "uid": 101,
            "scores": {  
                "shake_badge": 100,
                "dino": 200,
                "snake": 300,
                "tetris": 400,
                "connect_sponsor": 500,
                "rectf": 600
            },
            "total_score": 2100
        },
        {
            "name": "chen",
            "uid": 102,
            "scores": {
                "shake_badge": 200,
                "dino": 300,
                "snake": 400,
                "tetris": 500,
                "connect_sponsor": 600,
                "rectf": 700
            },
            "total_score": 2700
        },
        {
            "name": "sherry",
            "uid": 103,
            "scores": {
                "shake_badge": 300,
                "dino": 400,
                "snake": 500,
                "tetris": 600,
                "connect_sponsor": 700,
                "rectf": 800
            },
            "total_score": 3300
        }
    ]


## ====== Station API Endpoints ======
@router.get("/tx")
async def tx(station: Station = Depends(get_station)) -> list[IrPacketRequestSchema]:
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
    return station.display


@router.get("/station-score")
async def get_history(station: Station = Depends(get_station)):
    # Fetch the history of packets for the station
    history = await GameLogicController.get_station_score(station.station_id)
    return history


## ====== ReCTF API Endpoints ======
@app.post("/rectf/score")
async def receive_rectf_score(schema: ReCTFScoreSchema, credentials: HTTPAuthorizationCredentials = Security(security)):
    # Validate ReCTF key
    if credentials.credentials != config.get('rectf', {}).get('api_key'):
        raise HTTPException(status_code=403, detail="Invalid key")

    # Process the ReCTF score for the user
    await GameLogicController.apply_rectf_score(schema.uid, schema.solves)

    return {"status": "ok"}


## ====== Badge Linking API Endpoints ======
@app.post("/hitcon/link")
async def hitcon_link(schema: BadgeLinkSchema, credentials: HTTPAuthorizationCredentials = Security(security)):
    # Validate attendee token
    if not credentials.credentials:
        raise HTTPException(status_code=400, detail="Missing token")

    uid = await BadgeLinkController.get_uid_with_token(credentials.credentials)

    if not uid:
        raise HTTPException(status_code=401, detail="Invalid token")

    # Link the badge with the attendee
    await BadgeLinkController.link_badge_with_attendee(uid, schema.badge_user)

    # TODO: apply rectf buff if the user has not link with the badge

    return {"status": "ok"}


app.include_router(router)
