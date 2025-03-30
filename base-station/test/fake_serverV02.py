
from fastapi import FastAPI, Request
from pydantic import BaseModel
from typing import List, Union, Optional
import uuid
import uvicorn

app = FastAPI()

# 封包資料模型
class ProximityPayload(BaseModel):
    username: List[int]
    power: int
    nonce: List[int]
    sig: List[int]

class IRPacket(BaseModel):
    ttl: int
    type: int
    payload: Union[ProximityPayload, dict]  # 可依照 type 擴充更多型別

class IncomingPacket(BaseModel):
    station_id: str
    packet_id: str
    packet_data: IRPacket

@app.post("/v1/rx")
async def receive_packet(packet: IncomingPacket):
    print("📥 收到封包 from:", packet.station_id)
    print("📦 封包內容:", packet.packet_data)
    return {"status": "OK"}

@app.get("/v1/tx")
async def get_tx():
    # 模擬一個 ProximityPacket 回傳
    return {
        "packet_id": str(uuid.uuid4()),
        "packet_data": {
            "ttl": 5,
            "type": 4,
            "payload": {
                "username": [72, 67, 84, 70],  # HCTF
                "power": 99,
                "nonce": [1, 2],
                "sig": [0] * 16
            }
        }
    }

@app.get("/v1/station-display")
async def get_display():
    return {
        "color_1_bar": 20,
        "color_2_bar": 80,
        "winning_color": "color_2"
    }

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)