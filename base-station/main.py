from backend_interface import BackendInterface
from ir_interface import IrInterface
from packet_processor import PacketProcessor
from config import Config
import asyncio


async def main():
    config = Config("config.yaml")
    try:
        async with PacketProcessor() as processor:
            async with BackendInterface(config=config) as backend:
                async with IrInterface(config=config) as ir:
                    processor.backend = backend
                    processor.ir = ir
                    processor.start()
                    shutdown_event = asyncio.Event()
                    try:
                        # Run forever.
                        await shutdown_event.wait()
                    except asyncio.CancelledError:
                        print("\nShutting down gracefully...")
    except KeyboardInterrupt:
        print("\nReceived shutdown signal")
        

if __name__ == "__main__":
    asyncio.run(main())
