import os
import base64
MONGO_CONNECT_STRING: str = os.environ['MONGO_CONNECT_STRING']
MONGO_DATABASE_NAME: str = os.environ["MONGO_DATABASE_NAME"]
SERVER_PRIV_KEY: int = int(os.environ["SERVER_PRIV_KEY"])
SERVER_PUB_KEY: bytes = base64.b64decode(os.environ["SERVER_PUB_KEY"])