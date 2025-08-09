from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import base64
import database
import config
import ecc

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

    return {
        'cert': base64.b64encode(ecc.server_sign_pubkey(priv_key)).decode('utf-8')
    }

@app.post('/commit_board')
async def commit_key(data: BoardData):
    storage: database.Storage = await database.Storage.create(config.MONGO_CONNECT_STRING, config.MONGO_DATABASE_NAME)
    try:
        priv_key: bytes = decode_base64(data.priv_key)
        await database.commit_item(priv_key)
    finally:
        await storage.close()
