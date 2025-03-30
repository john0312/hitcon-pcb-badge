import aiohttp
import asyncio
import uuid
import random
import os

# 從環境變數取得 API URL & Station Key（可用 .env 設定）
BASE_URL = os.getenv("BACKEND_URL", "http://localhost:8000/v1")
STATION_KEY = os.getenv("FAKE_STATION_KEY", "fake_station_key_123")

HEADERS = {
    "Authorization": f"Bearer {STATION_KEY}",
    "Content-Type": "application/json"
}

async def send_fake_rx(session):
    """非同步發送 IR 接收數據到後端"""
    while True:
        try:
            payload = {
                "station_id": str(uuid.uuid4()),
                "packet_id": str(uuid.uuid4()),
                "packet_data": [random.randint(0, 255) for _ in range(4)]
            }
            async with session.post(f"{BASE_URL}/rx", json=payload, headers=HEADERS) as response:
                print(f" `/v1/rx` Response: {await response.json()}")

        except Exception as e:
            print(f" `/v1/rx` 發送失敗: {e}")

        await asyncio.sleep(random.randint(3, 10))  # 非同步等待

async def request_fake_tx(session):
    """非同步請求 IR 發送封包"""
    while True:
        try:
            async with session.get(f"{BASE_URL}/tx", headers=HEADERS) as response:
                print(f" `/v1/tx` Response: {await response.json()}")

        except Exception as e:
            print(f" `/v1/tx` 發送失敗: {e}")

        await asyncio.sleep(random.randint(5, 15))  # 非同步等待

async def request_fake_display(session):
    """非同步請求 LED 顯示數據"""
    while True:
        try:
            async with session.get(f"{BASE_URL}/station-display", headers=HEADERS) as response:
                print(f" `/v1/station-display` Response: {await response.json()}")

        except Exception as e:
            print(f" `/v1/station-display` 發送失敗: {e}")

        await asyncio.sleep(random.randint(10, 20))  # 非同步等待

async def run_fake_client():
    """主執行函式，並行測試所有 API"""
    async with aiohttp.ClientSession() as session:
        tasks = [
            send_fake_rx(session),
            request_fake_tx(session),
            request_fake_display(session)
        ]
        await asyncio.gather(*tasks)  # 同時執行所有測試

if __name__ == "__main__":
    asyncio.run(run_fake_client())  # 啟動 Fake Client
