from beanie import Document, init_beanie, Indexed
from motor.motor_asyncio import AsyncIOMotorClient, AsyncIOMotorDatabase
from typing import Any, Dict, Annotated, Optional
import asyncio
from pymongo.errors import DuplicateKeyError
import ecc

class PrivKeyExistsError(Exception):
    pass

class BoardData(Document):
    priv_key: bytes
    user_id: Annotated[bytes, Indexed(unique=True)]
    commit: bool

    def __repr__(self) -> str:
        return f"BoardData(priv_key={self.priv_key}, id={self.id})"

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

async def store_item(priv_key: bytes) -> Optional[BoardData]:
    data = BoardData(priv_key=priv_key, user_id=ecc.privkey_to_username(priv_key), commit=False)
    try:
        return await data.insert()
    except DuplicateKeyError:
        raise PrivKeyExistsError("Private key already exists in the database")

async def commit_item(priv_key: bytes):
    await BoardData.find_one(BoardData.user_id == ecc.privkey_to_username(priv_key)).update_one(
        {"$set": {BoardData.commit: True}}
    )

async def get_all() -> list[BoardData]:
    return await BoardData.find_all().to_list()

