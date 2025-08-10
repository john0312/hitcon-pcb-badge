import aiohttp
import asyncio
import uuid
import random
import os

# 環境變數設置
BASE_URL = os.getenv("BACKEND_URL", "http://127.0.0.1:8000")
STATION_KEY = os.getenv("BEARER_TOKEN", "key")

HEADERS = {
    "Authorization": f"Bearer {STATION_KEY}",
    "Content-Type": "application/json"
}

async def send_fake_rx(session):
    """發送 IR 接收封包到後端"""
    payload = {
        "station_id": str(uuid.uuid4()),
        "packet_id": str(uuid.uuid4()),
        "packet_data": [random.randint(0, 255) for _ in range(4)]
    }
    try:
        async with session.post(f"{BASE_URL}/v1/rx", json=payload, headers=HEADERS) as response:
            print(f"[POST /v1/rx] Response: {await response.json()}")
    except Exception as e:
        print(f"[POST /v1/rx] 發送失敗: {e}")

async def request_fake_tx(session):
    """請求後端發送封包"""
    try:
        async with session.get(f"{BASE_URL}/v1/tx", headers=HEADERS) as response:
            print(f"[GET /v1/tx] Response: {await response.json()}")
    except Exception as e:
        print(f"[GET /v1/tx] 請求失敗: {e}")

async def request_fake_display(session):
    """請求 LED 顯示數據"""
    try:
        async with session.get(f"{BASE_URL}/v1/station-display", headers=HEADERS) as response:
            print(f"[GET /v1/station-display] Response: {await response.json()}")
    except Exception as e:
        print(f"[GET /v1/station-display] 請求失敗: {e}")

async def get_scores(session):
    """獲取假數據分數"""
    try:
        async with session.get(f"{BASE_URL}/api/scores") as response:
            data = await response.json()
            print("[GET /api/scores] Response:", data)
    except Exception as e:
        print(f"[GET /api/scores] 請求失敗: {e}")

async def post_score(session):
    """插入假數據分數"""
    fake_data = {
        "name": "test_user",
        "uid": random.randint(100, 999),
        "scores": {
            "shake_badge": random.randint(0, 500),
            "dino": random.randint(0, 500),
            "snake": random.randint(0, 500),
            "tetris": random.randint(0, 500),
            "connect_sponsor": random.randint(0, 500),
            "rectf": random.randint(0, 500)
        }
    }
    fake_data["total_score"] = sum(fake_data["scores"].values())
    try:
        async with session.post(f"{BASE_URL}/api/scores", json=fake_data) as response:
            data = await response.json()
            print("[POST /api/scores] Response:", data)
    except Exception as e:
        print(f"[POST /api/scores] 插入失敗: {e}")

async def main():
    async with aiohttp.ClientSession() as session:
        tasks = [
            send_fake_rx(session),
            request_fake_tx(session),
            request_fake_display(session),
            get_scores(session),
            post_score(session)
        ]
        await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(main())