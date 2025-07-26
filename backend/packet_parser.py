from typing import Optional, Union
from io import BytesIO
from schemas import Event, ProximityEvent, PubAnnounceEvent, TwoBadgeActivityEvent, GameActivityEvent, ScoreAnnounceEvent, SingleBadgeActivityEvent, SponsorActivityEvent
from schemas import PacketType, IrPacket, IrPacketRequestSchema, IR_USERNAME_LEN
from ecc_utils import ECC_SIGNATURE_SIZE, ECC_PUBKEY_SIZE


class PacketParser:
    @staticmethod
    def parse_packet(ir_packet: IrPacket) -> Optional[Event]:
        packet_type = PacketParser.get_packet_type(ir_packet)
        packet_id = ir_packet.packet_id
        station_id = ir_packet.station_id

        buf = BytesIO(ir_packet.data)
        buf.read(1)  # Skip TTL
        buf.read(1)  # Skip the first byte (packet type)
        b2i = lambda x: int.from_bytes(x, 'little', signed=False)

        match packet_type:
            case PacketType.kProximity:
                # Proximity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                power = b2i(buf.read(1))
                nonce = b2i(buf.read(2))
                signature = buf.read(ECC_SIGNATURE_SIZE)
                return ProximityEvent(packet_id=packet_id, station_id=station_id, user=user, power=power, nonce=nonce, signature=signature)

            case PacketType.kPubAnnounce:
                # Public announce packet
                pubkey = buf.read(ECC_PUBKEY_SIZE)
                signature = buf.read(ECC_SIGNATURE_SIZE)
                return PubAnnounceEvent(packet_id=packet_id, station_id=station_id, pubkey=pubkey, signature=signature)

            case PacketType.kTwoBadgeActivity:
                # Two badge Activity packet
                user1 = b2i(buf.read(IR_USERNAME_LEN))
                user2 = b2i(buf.read(IR_USERNAME_LEN))
                game_data = buf.read(5)
                signature = buf.read(ECC_SIGNATURE_SIZE)
                return TwoBadgeActivityEvent(packet_id=packet_id, station_id=station_id, user1=user1, user2=user2, game_data=game_data, signature=signature)

            case PacketType.kScoreAnnounce:
                # Score announce packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                score = b2i(buf.read(4))
                signature = buf.read(ECC_SIGNATURE_SIZE)
                return ScoreAnnounceEvent(packet_id=packet_id, station_id=station_id, user=user, score=score, signature=signature)

            case PacketType.kSingleBadgeActivity:
                # Single badge activity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                event_type = b2i(buf.read(1))
                event_data = buf.read(3)
                signature = buf.read(ECC_SIGNATURE_SIZE)
                return SingleBadgeActivityEvent(packet_id=packet_id, station_id=station_id, user=user, event_type=event_type, event_data=event_data, signature=signature)

            case PacketType.kSponsorActivity:
                # Sponsor activity packet
                user = b2i(buf.read(IR_USERNAME_LEN))
                sponsor_id = b2i(buf.read(1))
                sponsor_data = buf.read(9)
                return SponsorActivityEvent(packet_id=packet_id, station_id=station_id, user=user, sponsor_id=sponsor_id, sponsor_data=sponsor_data)

            case _:
                # Unknown packet type
                return None


    @staticmethod
    def get_packet_type(ir_packet: Union[IrPacket, IrPacketRequestSchema]) -> Optional[PacketType]:
        # Determine the type of packet based on its contents.
        # This is a placeholder implementation and should be replaced with actual logic.
        raw_type = ir_packet.data[1]
        packet_type = PacketType(raw_type)
        if packet_type in PacketType:
            return packet_type
        else:
            return None