from typing import Optional
from schemas import IrPacket, Event, TwoBadgeActivityEvent, SponsorActivityEvent, PubAnnounceEvent
from schemas import EccPoint, EccPublicKey, EccPrivateKey, EccSignature
from database import db
from ecc_utils import ECC_SIGNATURE_SIZE, ecc_sign, ecc_verify, ecc_get_point_by_x


class UnsignedPacketError(Exception):
    pass


# This module is responsible for verifying & signing the packets
class CryptoAuth:
    # ===== Generic methods for any other layers =====
    @staticmethod
    async def get_pubkey_by_username(user: int) -> Optional[EccPublicKey]:
        pub_x = int((await db["users"].find_one({"user": user}))["pubkey"])

        if pub_x is None:
            return
        
        pub = abs(pub_x)
        sign = pub_x < 0

        pub = ecc_get_point_by_x(pub, sign)
        return EccPublicKey(point=EccPoint(x=pub.x, y=pub.y))


    @staticmethod
    async def derive_user_by_pubkey(pub: EccPublicKey) -> Optional[int]:
        pub_x = pub.point.x
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
            sig = CryptoAuth.parse_raw_signature(event.signature.to_bytes(14, 'little'))

            pub1 = await CryptoAuth.get_pubkey_by_username(event.user1)
            pub2 = await CryptoAuth.get_pubkey_by_username(event.user2)

            sig_user1 = EccSignature(**(sig.model_dump() | {"pub": pub1}))
            sig_user2 = EccSignature(**(sig.model_dump() | {"pub": pub2}))

            if ecc_verify(
                msg=ir_packet.data[2:ECC_SIGNATURE_SIZE],
                sig=sig_user1
            ):
                return event.user1
            elif ecc_verify(
                msg=ir_packet.data[2:ECC_SIGNATURE_SIZE],
                sig=sig_user2
            ):
                return event.user2
            else:
                raise UnsignedPacketError("Invalid signature for the packet")
        elif event.__class__ == SponsorActivityEvent:
            # SponsorActivityEvent does not require signature verification
            pass
        elif event.__class__ == PubAnnounceEvent:
            x = int.from_bytes(event.pubkey[:ECC_SIGNATURE_SIZE - 1], 'little', signed=False)
            sign = bool(event.pubkey[-1])

            p = ecc_get_point_by_x(x, sign)
            # TODO: verify last byte of x, which should only be 0 or 1
            pub = EccPublicKey(point=EccPoint(x=p.x, y=p.y))

            sig = CryptoAuth.parse_raw_signature(event.signature.to_bytes(14, 'little'))
            sig = EccSignature(**(sig.model_dump() | {"pub": pub}))
            
            if not ecc_verify(
                msg=ir_packet.data[2:ECC_SIGNATURE_SIZE],
                sig=sig
            ):
                raise UnsignedPacketError("Invalid signature for the public key")

            user = await CryptoAuth.derive_user_by_pubkey(pub)
            return user
        else:
            sig = CryptoAuth.parse_raw_signature(event.signature.to_bytes(14, 'little'))
            pub = await CryptoAuth.get_pubkey_by_username(event.user)

            sig = EccSignature(**(sig.model_dump() | {"pub": pub}))

            if not ecc_verify(
                msg=ir_packet.data[2:ECC_SIGNATURE_SIZE],
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