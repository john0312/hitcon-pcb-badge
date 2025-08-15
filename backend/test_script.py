import asyncio

from config import Config
from packet_processor import PacketProcessor
from crypto_auth import CryptoAuth
from schemas import EccPrivateKey, IrPacket, PacketType
from ecc_utils import ecc_sign


config = Config("config.yaml")
packet_processor_instance = PacketProcessor(config=config)


async def main():
    # add station
    await packet_processor_instance.stations.insert_one({"station_id": 1, "station_key": 'key'})

    # add user
    pub_bytes = b'TODO'
    pub = CryptoAuth.parse_pubkey_bytes(pub_bytes)
    user_id = await CryptoAuth.derive_user_by_pubkey(pub)
    if not user_id:
        user_id = await CryptoAuth.create_user(pub)

    # add sponsor
    pub_bytes = b'TODO'
    pub = CryptoAuth.parse_pubkey_bytes(pub_bytes)
    priv = 0 # TODO
    sponsor_user_id = await CryptoAuth.derive_user_by_pubkey(pub)
    if not sponsor_user_id:
        sponsor_user_id = await CryptoAuth.create_user(pub)
    await packet_processor_instance.users.update_one({"user": sponsor_user_id}, {"$set": {"sponsor_id": 1}}, upsert=True)

    # create packet
    pkt = IrPacket(
        data=b"".join([
            b"\x00",
            bytes([PacketType.kSponsorActivity.value]),
            b"\x01",
            b"\x01",
            user_id.to_bytes(4, 'little', signed=False)
        ]),
        station_id=1,
        to_stn=True
    )
    sig = ecc_sign(pkt.data, EccPrivateKey(dA=priv))
    pkt.data += sig.to_bytes()
    print(pkt)


if __name__ == "__main__":
    asyncio.run(main())

