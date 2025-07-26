from typing import Optional
from schemas import IrPacket, Event, TwoBadgeActivityEvent, ScoreAnnounceEvent, SponsorActivityEvent, PubAnnounceEvent
from schemas import EccPoint, EccPublicKey, EccPrivateKey, EccSignature
from database import db
from ecc_utils import ECC_SIGNATURE_SIZE, ecc_sign, ecc_derive_pub, ecc_verify, ecc_get_point_by_x
from config import Config

config = Config("config.yaml")

class UnsignedPacketError(Exception):
    pass


# This module is responsible for verifying & signing the packets
class CryptoAuth:
    # This is the private key of the server, used to sign packets
    server_key = EccPrivateKey(
        dA=config.get("backend", {}).get("ecc_key", 878787)
    )
    server_pub = ecc_derive_pub(server_key)

    # ===== Generic methods for any other layers =====
    @staticmethod
    async def get_pubkey_by_username(user: int) -> Optional[EccPublicKey]:
        user = (await db["users"].find_one({"user": user}))

        if user is None:
            return

        pub_x = int(user["pubkey"])

        if pub_x is None:
            return

        pub = abs(pub_x)
        sign = pub_x < 0

        pub = ecc_get_point_by_x(pub, sign)
        return EccPublicKey(point=EccPoint(x=pub.x, y=pub.y))


    @staticmethod
    async def derive_user_by_pubkey(pub: EccPublicKey) -> Optional[int]:
        # Last byte of compact form of x is stored as sign in database.
        pub_x = pub.point.x
        sign = pub.point.y % 2
        if sign:
            pub_x = -pub_x

        user = await db["users"].find_one({"pubkey": pub_x})

        if user is None:
            return None

        return user["user"]


    # ===== APIs for PacketProcessor =====
    @staticmethod
    async def verify_packet(event: Event, ir_packet: IrPacket) -> Optional[int]:
        """
        Verify the packet. Throws an exception if the packet is invalid.
        Returns username if the packet is valid.
        """
        if event.__class__ == TwoBadgeActivityEvent:
            sig = CryptoAuth.parse_raw_signature(event.signature)

            pub1 = await CryptoAuth.get_pubkey_by_username(event.user1)
            pub2 = await CryptoAuth.get_pubkey_by_username(event.user2)

            sig_user1 = EccSignature(**(sig.model_dump() | {"pub": pub1}))
            sig_user2 = EccSignature(**(sig.model_dump() | {"pub": pub2}))

            if ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=sig_user1
            ):
                event.packet_from = 1
                return event.user1
            elif ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=sig_user2
            ):
                event.packet_from = 2
                return event.user2
            else:
                raise UnsignedPacketError("Invalid signature for the packet")
        elif event.__class__ == SponsorActivityEvent or event.__class__ == ScoreAnnounceEvent:
            # SponsorActivityEvent, ScoreAnnounceEvent does not require signature verification
            pass
        elif event.__class__ == PubAnnounceEvent:
            x = int.from_bytes(event.pubkey[:ECC_SIGNATURE_SIZE - 1], 'little', signed=False)
            sign = bool(event.pubkey[-1])

            p = ecc_get_point_by_x(x, sign)
            # TODO: verify last byte of x, which should only be 0 or 1
            pub = EccPublicKey(point=EccPoint(x=p.x, y=p.y))

            sig = CryptoAuth.parse_raw_signature(event.signature)
            sig = EccSignature(**(sig.model_dump() | {"pub": pub}))
            
            if not ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=sig
            ):
                raise UnsignedPacketError("Invalid signature for the public key")

            user = await CryptoAuth.derive_user_by_pubkey(pub)
            return user
        else:
            sig = CryptoAuth.parse_raw_signature(event.signature)
            pub = await CryptoAuth.get_pubkey_by_username(event.user)

            sig = EccSignature(**(sig.model_dump() | {"pub": pub}))

            if not ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=sig
            ):
                raise UnsignedPacketError("Invalid signature for the packet")

        return event.user


    @staticmethod
    def parse_raw_signature(raw_sig: bytes) -> EccSignature:
        """
        Parse the raw signature bytes into EccSignature.
        The raw signature is expected to be 14 bytes long.
        """
        if len(raw_sig) != 14:
            raise ValueError("Raw signature must be 14 bytes long")

        r = int.from_bytes(raw_sig[:7], 'little', signed=False)
        s = int.from_bytes(raw_sig[7:14], 'little', signed=False)
        
        return EccSignature(r=r, s=s, pub=None)


    # ===== APIs for GameLogic =====
    @staticmethod
    async def get_user_team(user: int) -> int:
        """
        Return team as sign.
        """
        key = await CryptoAuth.get_pubkey_by_username(user)

        sign = key.point.y % 2
        if sign == 0:
            return -1
        else:
            return 1