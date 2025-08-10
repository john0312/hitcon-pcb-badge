import base64
import json
import requests
import config

class PrivKeyExistsException(Exception):
    pass

class InternalServerError(Exception):
    pass

class UnknownError(Exception):
    pass

def encode_b64(data: bytes) -> str:
    return base64.b64encode(data).decode('utf-8')

def decode_b64(data: bytes | str) -> bytes:
    return base64.b64decode(data)

def post_data(url: str, data: dict):
    print(f'posting {data}')
    response = requests.post(url, json.dumps(data))

    if response.status_code == 409:
        raise PrivKeyExistsException()
    elif response.status_code == 500:
        raise InternalServerError()
    elif response.status_code != 200:
        raise UnknownError(f'Unknown status code{response.status_code}')

    return response.text

def post_board_data(priv_key: bytes) -> bytes:
    response = post_data(
        f'{config.POST_URL}/log_board',
        {'priv_key': encode_b64(priv_key)}
    )

    return decode_b64(json.loads(response)['cert'])

def post_commit_privkey(priv_key: bytes):
    post_data(
        f'{config.POST_URL}/commit_board',
        {'priv_key': encode_b64(priv_key)}
    )