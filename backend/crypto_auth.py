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
    def parse_pubkey(pub_x: int) -> EccPublicKey:
        pub = abs(pub_x)
        sign = pub_x < 0
        return EccPublicKey(point=ecc_get_point_by_x(pub, sign))


    @staticmethod
    def encode_pubkey(pub: EccPublicKey) -> int:
        x = pub.point.x
        sign = pub.point.y % 2
        if sign:
            x = -x
        return x


    @staticmethod
    async def get_pubkey_by_username(user: int) -> Optional[EccPublicKey]:
        u = await db["users"].find_one({"user": user})

        if u is None:
            return

        return CryptoAuth.parse_pubkey(u["pubkey"])


    @staticmethod
    async def get_pubkeys_by_sponsor_id(sponsor_id: int) -> list[EccPublicKey]:
        """
        Get the public keys of the sponsor by sponsor_id.
        """
        sponsors = db["users"].find({"sponsor_id": sponsor_id})

        return [CryptoAuth.parse_pubkey(s["pubkey"]) async for s in sponsors]


    @staticmethod
    async def derive_user_by_pubkey(pub: EccPublicKey) -> Optional[int]:
        # Last byte of compact form of x is stored as sign in database.
        pub_x = CryptoAuth.encode_pubkey(pub)

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
            if ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=EccSignature.from_bytes(
                    event.signature,
                    pub=await CryptoAuth.get_pubkey_by_username(event.user1)
                )
            ):
                event.packet_from = 1
                return event.user1
            elif ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=EccSignature.from_bytes(
                    event.signature,
                    pub=await CryptoAuth.get_pubkey_by_username(event.user2)
                )
            ):
                event.packet_from = 2
                return event.user2
            else:
                raise UnsignedPacketError("Invalid signature for the packet")
        elif event.__class__ == SponsorActivityEvent:
            # Verify SponsorActivityEvent with Sponsor's public key according to the sponsor_id
            for pub in await CryptoAuth.get_pubkeys_by_sponsor_id(event.sponsor_id):
                if ecc_verify(
                    msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                    sig=EccSignature.from_bytes(
                        event.signature,
                        pub=pub
                    )
                ):
                    return event.user
        elif event.__class__ == ScoreAnnounceEvent:
            # ScoreAnnounceEvent does not require signature verification
            pass
        elif event.__class__ == PubAnnounceEvent:
            # Validate the public key with server key (CA)
            sig = EccSignature.from_bytes(event.signature, pub=CryptoAuth.server_pub)

            if not ecc_verify(
                msg=event.pubkey,
                sig=sig
            ):
                raise UnsignedPacketError("Invalid signature for the public key")

            x = int.from_bytes(event.pubkey[:ECC_SIGNATURE_SIZE - 1], 'little', signed=False)
            sign = bool(event.pubkey[-1])

            p = ecc_get_point_by_x(x, sign)
            pub = EccPublicKey(point=EccPoint(x=p.x, y=p.y))

            user = await CryptoAuth.derive_user_by_pubkey(pub)
            return user
        else:
            pub = await CryptoAuth.get_pubkey_by_username(event.user)
            sig = EccSignature.from_bytes(event.signature, pub=pub)

            if not ecc_verify(
                msg=ir_packet.data[2:-ECC_SIGNATURE_SIZE],
                sig=sig
            ):
                raise UnsignedPacketError("Invalid signature for the packet")

        return event.user


    @staticmethod
    def sign_packet(ir_packet: IrPacket) -> IrPacket:
        """
        Sign the packet with the server's private key.
        """
        sig = ecc_sign(
            msg=ir_packet.data[2:],
            priv=CryptoAuth.server_key
        )

        ir_packet.data = ir_packet.data + sig.to_bytes()
        return ir_packet


    # ===== APIs for GameLogic =====
    @staticmethod
    async def get_user_team(user: int) -> int:
        """
        Return team as sign.
        """
        key = await CryptoAuth.get_pubkey_by_username(user)

        if key is None:
            raise ValueError(f"User {user} not found")

        sign = key.point.y % 2
        if sign == 0:
            return -1
        else:
            return 1


    @staticmethod
    async def create_user(pubkey: EccPublicKey) -> int:
        """
        Create a user with the given public key.
        Returns the user ID.
        """
        user = pubkey.point.x.to_bytes(7, 'little', signed=False)[3:6]

        x = CryptoAuth.encode_pubkey(pubkey)

        # Create a new user
        await db["users"].insert_one({"user": user, "pubkey": x, "station_id": None})

        return user
