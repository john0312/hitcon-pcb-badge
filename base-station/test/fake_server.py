from fastapi import FastAPI
from pydantic import BaseModel
import uuid

app = FastAPI()

# 定義 IR 接收數據的結構
class IRPacket(BaseModel):
    station_id: str
    packet_id: str
    packet_data: list[int]

# 1. /v1/tx (Fake Client 來這裡請求 IR 發送封包)
@app.get("/v1/tx")
async def get_tx():
    return {"packet_id": str(uuid.uuid4()), "packet_data": [0x12, 0x34, 0x56, 0x78]}

# 2. /v1/rx (Fake Client 來這裡發送 IR 接收的數據)
@app.post("/v1/rx")
async def post_rx(packet: IRPacket):
    print(f"📡 Fake Client 送來的 IR 數據: {packet}")
    return {"status": "OK"}

#  3. /v1/station-display (Fake Client 來這裡請求顯示數據)
@app.get("/v1/station-display")
async def get_display():
    return {"color_1_bar": 50, "color_2_bar": 30, "winning_color": "color_1"}

#  啟動 Fake Server
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
