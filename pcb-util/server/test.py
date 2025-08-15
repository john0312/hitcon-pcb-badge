import requests
import base64
import random


URL = 'http://localhost:8080'
CURVE_ORDER = 0xbcffb09c43733d

def log_board() -> bytes:
    privkey = random.randint(1, CURVE_ORDER).to_bytes(7, 'little')
    payload = {'priv_key': base64.b64encode(privkey).decode('utf-8')}
    print(f'logging {payload}')
    requests.post(f'{URL}/log_board', json=payload)
    return privkey

def commit_board(privkey: bytes):
    payload = {'priv_key': base64.b64encode(privkey).decode('utf-8')}
    print(f'committing {payload}')
    requests.post(f'{URL}/commit_board', json=payload)

def get_boards():
    return requests.get(f'{URL}/get_boards').text

if __name__ == '__main__':
    privkey = log_board()
    commit_board(privkey)
    print(get_boards())


