import uuid
import aiohttp
import asyncio
from config import Config
from typing import Tuple, Optional


class BackendInterface:
    def __init__(self, config: Config):
        self.station_id = config.get("station_id")
        self.backend_url = config.get("backend_url")
        self.station_key = config.get("station_key")
        self.session = aiohttp.ClientSession()
        self.retry_delay = 2

    async def send_received_packet(self, data: bytes, packet_id: uuid.UUID) -> bool:
        payload = {
            "station_id": self.station_id,
            "packet_id": str(packet_id),
            "packet_data": list(data)
        }
        url = f"{self.backend_url}/rx"
        headers = {"Authorization": f"Bearer {self.station_key}"}
        try:
            async with self.session.post(url, json=payload, headers=headers) as resp:
                print(f"[RX] POST /rx status: {resp.status}")
                return resp.status == 200
        except Exception as e:
            print(f"[RX] 發送失敗: {e}")
            return False

    async def get_next_tx_packet(self) -> Optional[Tuple[bytes, uuid.UUID]]:
        url = f"{self.backend_url}/tx"
        headers = {"Authorization": f"Bearer {self.station_key}"}
        try:
            async with self.session.get(url, headers=headers) as resp:
                if resp.status == 200:
                    result = await resp.json()
                    packet_id = uuid.UUID(result["packet_id"])
                    packet_data = bytes(result["packet_data"])
                    return packet_data, packet_id
                else:
                    print("[TX] 無新封包")
                    return None
        except Exception as e:
            print(f"[TX] 輪詢失敗: {e}")
            return None

    async def _tx_poll_task(self):
        while True:
            try:
                result = await self.get_next_tx_packet()
                if result:
                    packet_data, packet_id = result
                    print(f"[TX] 收到封包 {packet_id}: {packet_data}")
                    # TODO: 呼叫 IR 發射器
                await asyncio.sleep(3)
            except Exception as e:
                print(f"[TX] 輪詢錯誤，將在 {self.retry_delay}s 後重試：{e}")
                await asyncio.sleep(self.retry_delay)

    async def close(self):
        await self.session.close()