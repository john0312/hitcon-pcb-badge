import uuid
import aiohttp
import asyncio
from config import Config
from typing import Tuple, Optional


class BackendInterface:
    def __init__(self, config: Config):
        self.station_id = config.get(key="station_id")
        self.config = config

    async def send_received_packet(self, data: bytes, packet_id: uuid.UUID):
        # 發送接收到的 IR 封包到後端 (/v1/rx)
        async with aiohttp.ClientSession() as session:
            payload = {
                "station_id": self.station_id,
                "packet_id": str(packet_id),
                "packet_data": list(data)
            }
            try:
                async with session.post(
                    f"{self.config.get('backend_url')}/v1/rx",
                    json=payload,
                    headers={"Authorization": f"Bearer {self.config.get('station_key')}"}
                ) as resp:
                    print(f"[RX] 回應狀態碼: {resp.status}")
                    return await resp.json()
            except Exception as e:
                print(f"[RX] 發送失敗: {e}")
                return None

    async def get_next_tx_packet(self) -> Optional[Tuple[bytes, uuid.UUID]]:
        # 發送輪詢請求到後端 (/v1/tx)
        async with aiohttp.ClientSession() as session:
            try:
                async with session.get(
                    f"{self.config.get('backend_url')}/v1/tx",
                    headers={"Authorization": f"Bearer {self.config.get('station_key')}"}
                ) as resp:
                    if resp.status == 200:
                        result = await resp.json()
                        packet_id = uuid.UUID(result["packet_id"])
                        packet_data = bytes(result["packet_data"])
                        return packet_data, packet_id
                    else:
                        print("[TX] 沒有新封包")
                        return None
            except Exception as e:
                print(f"[TX] 輪詢失敗: {e}")
                return None

    async def _tx_poll_task(self):
        # 長輪詢任務：定期檢查是否有可發送的封包
        while True:
            result = await self.get_next_tx_packet()
            if result:
                packet_data, packet_id = result
                print(f"[TX] 要發送的封包: {packet_id} -> {packet_data}")
                # 在這裡你可以呼叫 IR 發射模組
            await asyncio.sleep(3)
