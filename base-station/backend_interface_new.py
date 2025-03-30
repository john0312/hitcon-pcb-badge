import uuid
import aiohttp
import asyncio
from config import Config
from typing import Tuple, Optional


class BackendInterface:
    def __init__(self, config: Config):
        self.station_id = config.get("station_id")
        self.config = config

    async def send_received_packed(self, data: bytes, packet_id: uuid.UUID) -> bool:
        async with aiohttp.ClientSession() as session:
            payload = {
                "station_id": self.station_id,
                "packet_id": str(packet_id),
                "packet_data": list(data)
            }
            try:
                async with session.post(
                    f"{self.config.get('backend_url')}/rx",
                    json=payload,
                    headers={"Authorization": f"Bearer {self.config.get('station_key')}"}
                ) as resp:
                    print(f"[RX] POST /rx status: {resp.status}")
                    return resp.status == 200
            except Exception as e:
                print(f"[RX] 發送失敗: {e}")
                return False

    async def get_next_tx_packet(self) -> Optional[Tuple[bytes, uuid.UUID]]:
        async with aiohttp.ClientSession() as session:
            try:
                async with session.get(
                    f"{self.config.get('backend_url')}/tx",
                    headers={"Authorization": f"Bearer {self.config.get('station_key')}"}
                ) as resp:
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
            result = await self.get_next_tx_packet()
            if result:
                packet_data, packet_id = result
                print(f"[TX] 收到封包 {packet_id}: {packet_data}")
                # 可呼叫 IR 發射邏輯
            await asyncio.sleep(3)
