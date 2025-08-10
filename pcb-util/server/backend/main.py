from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import base64
import database
import config

app = FastAPI()

class BoardData(BaseModel):
    board_secret: str
    priv_key: str

def decode_base64(data: str) -> bytes:
    try:
        return base64.b64decode(data)
    except:
        raise HTTPException(status_code=400, detail="Invalid base64 encoding")


@app.post('/log_board')
async def log_board(data: BoardData):
    storage: database.Storage = await database.Storage.create(config.MONGO_CONNECT_STRING, config.MONGO_DATABASE_NAME)
    try:
        board_secret: bytes = decode_base64(data.board_secret)
        priv_key: bytes = decode_base64(data.priv_key)
        await database.store_item(board_secret, priv_key)
    except database.PrivKeyExistsError as e:
        raise HTTPException(status_code=409, detail=str(e))
    finally:
        await storage.close()
