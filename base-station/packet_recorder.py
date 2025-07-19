import json
import uuid
import os
from datetime import datetime
from pathlib import Path

class PacketRecorder:
    def __init__(self, log_dir="./packet_log"):
        self.log_dir = Path(log_dir)
        os.makedirs(self.log_dir, exist_ok=True)

    def _get_timestamp(self):
        return datetime.now().timestamp()

    def _format_datetime(self, ts):
        return datetime.fromtimestamp(ts).strftime("%Y%m%d_%H%M%S")

    async def record_packet(self, data: bytes, direction: str, packet_id: uuid.UUID):
        """Record a packet to log file"""
        ts = self._get_timestamp()
        log_entry = {
            "datetime": ts,
            "data": data.hex(),
            "direction": direction,
            "packet_id": str(packet_id)
        }
        
        filename = f"{direction}-{self._format_datetime(ts)}-{packet_id}.json"
        filepath = self.log_dir / filename
        
        with open(filepath, "w") as f:
            json.dump(log_entry, f, indent=2)
