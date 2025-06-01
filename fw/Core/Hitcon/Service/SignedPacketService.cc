#include <Service/SignedPacketService.h>
#include <string.h>

namespace hitcon {

SignedPacketService g_signed_packet_service;

namespace signed_packet {

SignedPacket::SignedPacket() : status(kFree) {}

}  // namespace signed_packet

using namespace hitcon::signed_packet;

SignedPacketService::SignedPacketService()
    : routineTask(950, (callback_t)&SignedPacketService::RoutineFunc, this,
                  500) {}

void SignedPacketService::Init() {
  hitcon::service::sched::scheduler.Queue(&routineTask, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routineTask);
}

static bool getPacketSigInfo(packet_type packetType, size_t &sigOffset,
                             size_t &dataSize) {
  switch (packetType) {
    case packet_type::kProximity:
      sigOffset = offsetof(hitcon::ir::ProximityPacket, sig);
      dataSize = sizeof(hitcon::ir::ProximityPacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::ProximityPacket) - ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    case packet_type::kPubAnnounce:
      sigOffset = offsetof(hitcon::ir::PubAnnouncePacket, sig);
      dataSize = sizeof(hitcon::ir::PubAnnouncePacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::PubAnnouncePacket) -
                        ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    case packet_type::kTwoBadgeActivity:
      sigOffset = offsetof(hitcon::ir::TwoBadgeActivityPacket, sig);
      dataSize =
          sizeof(hitcon::ir::TwoBadgeActivityPacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::TwoBadgeActivityPacket) -
                        ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    case packet_type::kScoreAnnonce:
      sigOffset = offsetof(hitcon::ir::ScoreAnnouncePacket, sig);
      dataSize = sizeof(hitcon::ir::ScoreAnnouncePacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::ScoreAnnouncePacket) -
                        ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    case packet_type::kSingleBadgeActivity:
      sigOffset = offsetof(hitcon::ir::SingleBadgeActivityPacket, sig);
      dataSize =
          sizeof(hitcon::ir::SingleBadgeActivityPacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::SingleBadgeActivityPacket) -
                        ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    default:
      return false;
  }
  return true;
}

/**
 * Find an empty slot and send it for singing.
 * The signing doesn't occur instantly. Instead, GameController waits until ECC
 * is ready and queues the job.
 */
bool SignedPacketService::SignAndSendData(packet_type packetType,
                                          const uint8_t *data, size_t size) {
  size_t packetId;
  for (packetId = 0; packetId < PACKET_QUEUE_SIZE; ++packetId) {
    if (packet_queue_[packetId].status == PacketStatus::kFree) break;
  }
  if (packetId == PACKET_QUEUE_SIZE) return false;

  SignedPacket &packet = packet_queue_[packetId];
  size_t sigOffset, sizeReq;
  if (!getPacketSigInfo(packetType, sigOffset, sizeReq)) return false;
  if (size != sizeReq) return false;
  packet.signatureOffset = sigOffset;
  packet.type = packetType;
  memcpy(packet.data, data, size);
  packet.dataSize = size;
  packet.status = kWaitSignStart;
  return true;
}

void SignedPacketService::OnPacketSignFinish(
    hitcon::ecc::Signature *signature) {
  SignedPacket &packet = packet_queue_[signingPacketId];
  signature->toBuffer(packet.sig);
  packet.status = PacketStatus::kWaitTransmit;
}

void SignedPacketService::RoutineFunc() {
  for (size_t packetId = 0; packetId < PACKET_QUEUE_SIZE; ++packetId) {
    if (packet_queue_[packetId].status != kWaitSignStart) continue;
    SignedPacket &packet = packet_queue_[packetId];
    bool ret = hitcon::ecc::g_ec_logic.StartSign(
        packet.data, packet.dataSize,
        (callback_t)&SignedPacketService::OnPacketSignFinish, this);
    if (!ret) break;
    signingPacketId = packetId;
  }
  // Scan the packet queue and transmit them if possible
  for (size_t packetId = 0; packetId < PACKET_QUEUE_SIZE; ++packetId) {
    if (packet_queue_[packetId].status != kWaitTransmit) continue;
    SignedPacket &packet = packet_queue_[packetId];
    hitcon::ir::IrData irdata = {.ttl = 0, .type = packet.type};
    memcpy(&irdata.opaq, packet.data, packet.dataSize);
    memcpy(reinterpret_cast<uint8_t *>(&irdata.opaq) + packet.signatureOffset,
           packet.sig, sizeof(packet.sig));
    bool ret = hitcon::ir::irController.SendPacketWithRetransmit(
        reinterpret_cast<uint8_t *>(&irdata),
        packet.dataSize + ECC_SIGNATURE_SIZE, 3,
        ::hitcon::ir::AckTag::ACK_TAG_NONE);
    packet.status = PacketStatus::kFree;
  }
}

}  // namespace hitcon
