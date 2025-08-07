from beanie import Document, init_beanie, Indexed
from motor.motor_asyncio import AsyncIOMotorClient, AsyncIOMotorDatabase
from typing import Any, Dict, Annotated, Optional
import asyncio
from pymongo.errors import DuplicateKeyError

class PrivKeyExistsError(Exception):
    pass

class BoardData(Document):
    board_secret: bytes
    priv_key: Annotated[bytes, Indexed(unique=True)]


    def __repr__(self) -> str:
        return f"BoardData(board_secret={self.board_secret}, priv_key={self.priv_key}, id={self.id})"

class Storage:
    def __init__(self, connection_string: str, database_name: str):
        self.connection_string = connection_string
        self.database_name = database_name

    @classmethod
    async def create(cls, connection_string: str, database_name: str):
        instance = cls(connection_string, database_name)
        await instance._create()
        return instance

    async def _create(self) -> None:
        self.client: AsyncIOMotorClient[Dict[str, Any]] = AsyncIOMotorClient(self.connection_string)
        self.database: AsyncIOMotorDatabase[Dict[str, Any]] = self.client[self.database_name]

        await init_beanie(database=self.database, # type: ignore[arg-type]
                          document_models=[BoardData])

    async def close(self) -> None:
        self.client.close()

async def store_item(board_secret: bytes, priv_key: bytes) -> Optional[BoardData]:
    data = BoardData(board_secret=board_secret, priv_key=priv_key)
    try:
        return await data.insert()
    except DuplicateKeyError:
        raise PrivKeyExistsError("Private key already exists in the database")

async def main():
    storage: Storage = await Storage.create("mongodb://localhost:27017", "board_logs")
    try:
        stored_item = await store_item(b'aoeu', b'htns')
        print(stored_item)
    finally:
        await storage.close()


if __name__ == '__main__':
    asyncio.run(main())

