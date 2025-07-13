import uuid, json
import aiohttp
import asyncio
from config import Config
from typing import Tuple, Optional


class BackendInterface:
    REQUEST_TIMEOUT = 5  # seconds
    def __init__(self, config: Config):
        self.station_id = config.get("station_id")
        self.backend_url = config.get("backend_url")
        self.station_key = config.get("station_key")
        self.session = None
        self.retry_delay = 2

    async def __aenter__(self):
        self.session = aiohttp.ClientSession()
        return self

    async def __aexit__(self, exc_type, exc, tb):
        if self.session:
            await self.session.close()

    async def send_received_packet(self, data: bytes, packet_id: uuid.UUID) -> bool:
        payload = {
            "station_id": self.station_id,
            "packet_id": str(packet_id),
            "data": list(data)
        }
        url = f"{self.backend_url}/rx"
        headers = {"Authorization": f"Bearer {self.station_key}"}
        try:
            assert self.session is not None, "Session not initialized"
            timeout = aiohttp.ClientTimeout(total=self.REQUEST_TIMEOUT)
            async with self.session.post(url, json=payload, headers=headers, timeout=timeout) as resp:
                print(f"[RX] POST /rx status: {resp.status}")
                return resp.status == 200
        except Exception as e:
            print(f"[RX] Send failed: {e}")
            return False

    async def get_next_tx_packet(self) -> Optional[Tuple[bytes, uuid.UUID]]:
        url = f"{self.backend_url}/tx"
        headers = {"Authorization": f"Bearer {self.station_key}"}
        try:
            assert self.session is not None, "Session not initialized"
            timeout = aiohttp.ClientTimeout(total=self.REQUEST_TIMEOUT)
            async with self.session.get(url, headers=headers, timeout=timeout) as resp:
                if resp.status == 200:
                    result = await resp.json()
                    assert type(result) == list, "Expected a list of packets"
                    ret = []
                    for packet in result:
                        packet_id = uuid.UUID(packet["packet_id"])
                        packet_data = bytes(packet["data"])
                        ret.append((packet_data, packet_id))
                    return ret
                else:
                    print(f"[TX] Error fetching packet: {url} {resp.status} - {resp.reason}")
                    raise Exception(f"Failed to fetch packet: {resp.status} - {resp.reason}")
        except Exception as e:
            print(f"[TX] Polling failed: {e}")
            raise

    async def get_station_score(self) -> Optional[int]:
        url = f"{self.backend_url}/station-score"
        headers = {"Authorization": f"Bearer {self.station_key}"}
        try:
            assert self.session is not None, "Session not initialized for getting station score"
            timeout = aiohttp.ClientTimeout(total=self.REQUEST_TIMEOUT)
            async with self.session.get(url, headers=headers, timeout=timeout) as resp:
                if resp.status == 200:
                    result = await resp.json()
                    assert type(result) == int, "Expected a number"
                    #print(f"Got score {result}")
                    return result
                else:
                    print(f"[TX] Error fetching station score: {url} {resp.status} - {resp.reason}")
                    raise Exception(f"Failed to fetching station score: {resp.status} - {resp.reason}")
        except Exception as e:
            # Priint stack trace.
            import traceback
            traceback.print_exc()
            print(f"[TX] Polling station score failed: {e}")
            raise

