from typing import Optional
from schemas import IrPacket, Event, TwoBadgeActivityEvent, SponsorActivityEvent, PubAnnounceEvent
from database import db
from ecc_utils import ecc_sign, ecc_verify


class UnsignedPacketError(Exception):
    pass


# This module is responsible for verifying & signing the packets
class CryptoAuth:
    # ===== Generic methods for any other layers =====
    @staticmethod
    async def get_pubkey_by_username(user: int) -> Optional[int]:
        return db["users"].find_one({"user": user})["pubkey"]


    @staticmethod
    async def derive_user_by_pubkey(pubkey: bytes) -> Optional[int]:
        # TODO: mock fetch user
        return int.from_bytes(pubkey[:4], "little")


    # ===== APIs for PacketProcessor =====
    @staticmethod
    async def verify_packet(event: Event, ir_packet: IrPacket, packet_hash: bytes) -> Optional[int]:
        """
        Verify the packet. Throws an exception if the packet is invalid.
        Returns username if the packet is valid.
        """
        if event.__class__ == TwoBadgeActivityEvent:
            sig = event.signature.to_bytes(14, 'little')
            pub1 = (await CryptoAuth.get_pubkey_by_username(event.user1)).to_bytes(8, 'little')
            pub2 = (await CryptoAuth.get_pubkey_by_username(event.user2)).to_bytes(8, 'little')

            if MockECC.verify(
                sig=sig,
                pub=pub1,
                hash=packet_hash
            ):
                return event.user1
            elif MockECC.verify(
                sig=sig,
                pub=pub2,
                hash=packet_hash
            ):
                return event.user2
            else:
                raise UnsignedPacketError("Invalid signature for the packet")
        elif event.__class__ == SponsorActivityEvent:
            # SponsorActivityEvent does not require signature verification
            pass
        elif event.__class__ == PubAnnounceEvent:
            user = await CryptoAuth.derive_user_by_pubkey(event.pubkey.to_bytes(8, 'little'))
            return user
        else:
            sig = event.signature.to_bytes(14, 'little')
            pub = (await CryptoAuth.get_pubkey_by_username(event.user)).to_bytes(8, 'little')

            if not MockECC.verify(
                sig=sig,
                pub=pub,
                hash=packet_hash
            ):
                raise UnsignedPacketError("Invalid signature for the packet")

        return event.user

# MOCK ECC:

# PUBKEY[0:8] = PRIVKEY[0:8] ^ {0x12, 0x35, 0x57, 0x7a, 0xbd, 0xf9, 0xbf}
# SIG[0:14] = (PRIVKEY[0:8] ^ HASH[0:8] ^ {0x5, 0x3f, 0x85, 0x5c, 0xba, 0x24, 0x64, 0x44}) + (PRIVKEY[0:6] ^ HASH[2:8] ^ {0x18, 0x3b, 0xf6, 0x78, 0x37, 0x60})


class MockECC:
    @staticmethod
    def xor_bytes(a: bytes, b: bytes) -> bytes:
        if len(a) != len(b):
            raise ValueError("Byte strings must be of the same length")
        return bytes(x ^ y for x, y in zip(a, b))


    @staticmethod
    def derive_pub(priv: bytes) -> bytes:
        if len(priv) != 8:
            raise ValueError("Private key must be 8 bytes long")
        return MockECC.xor_bytes(priv, [0x12, 0x35, 0x57, 0x7a, 0xbd, 0xf9, 0xbf, 0x00])


    @staticmethod
    def sign(msg_b: bytes, priv_b: bytes) -> bytes:
        MockECC.xor_bytes(MockECC.xor_bytes(priv_b, msg_b), [0x5, 0x3f, 0x85, 0x5c, 0xba, 0x24, 0x64, 0x44]) + \
        MockECC.xor_bytes(MockECC.xor_bytes(priv_b[:6], msg_b[2:8]), [0x18, 0x3b, 0xf6, 0x78, 0x37, 0x60])


    @staticmethod
    def verify(sig: bytes, pub: bytes, hash: bytes) -> bool:
        expected_sig = MockECC.sign(hash, MockECC.derive_pub(pub))

        return sig == expected_sig