#include <App/ShowNameApp.h>
#include <App/TamaApp.h>
#include <Service/SignedPacketService.h>
#include <string.h>

namespace hitcon {

SignedPacketService g_signed_packet_service;

namespace signed_packet {

SignedPacket::SignedPacket() : status(kFree) {}

}  // namespace signed_packet

using namespace hitcon::signed_packet;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
SignedPacketService::SignedPacketService()
    : sigRoutineTask(950, (callback_t)&SignedPacketService::SigRoutineFunc,
                     this, 500),
      verRoutineTask(950, (callback_t)&SignedPacketService::VerRoutineFunc,
                     this, 500) {}
#pragma GCC diagnostic pop

void SignedPacketService::Init() {
  hitcon::service::sched::scheduler.Queue(&sigRoutineTask, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&sigRoutineTask);
  hitcon::service::sched::scheduler.Queue(&verRoutineTask, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&verRoutineTask);
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
    case packet_type::kScoreAnnounce:
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
    case packet_type::kSavePet:
      sigOffset = offsetof(hitcon::ir::SavePetPacket, sig);
      dataSize = sizeof(hitcon::ir::SavePetPacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::SavePetPacket) - ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    case packet_type::kRestorePet:
      sigOffset = offsetof(hitcon::ir::RestorePetPacket, sig);
      dataSize = sizeof(hitcon::ir::RestorePetPacket) - ECC_SIGNATURE_SIZE;
      static_assert(sizeof(hitcon::ir::RestorePetPacket) - ECC_SIGNATURE_SIZE <=
                    MAX_PACKET_DATA_SIZE);
      break;
    default:
      return false;
  }
  return true;
}

bool SignedPacketService::FindPacketOfState(SignedPacket *queue,
                                            PacketStatus status,
                                            size_t &packetId) {
  for (packetId = 0; packetId < PACKET_QUEUE_SIZE; ++packetId) {
    if (queue[packetId].status == status) return true;
  }
  return false;
}

bool SignedPacketService::VerifyAndReceivePacket(
    hitcon::ir::IrPacket *irpacket) {
  size_t packetId;
  hitcon::ir::IrData *irdata =
      reinterpret_cast<hitcon::ir::IrData *>(&(irpacket->data_[1]));
  if (!FindPacketOfState(ver_packet_queue_, PacketStatus::kFree, packetId))
    return false;

  SignedPacket &packet = ver_packet_queue_[packetId];
  packet_type packetType = irdata->type;
  size_t sigOffset, sizeReq;
  if (!getPacketSigInfo(packetType, sigOffset, sizeReq)) return false;
  packet.signatureOffset = sigOffset;
  packet.type = packetType;
  uint8_t *opaq_start = reinterpret_cast<uint8_t *>(&irdata->opaq);
  memcpy(packet.data, opaq_start, sizeReq);
  packet.dataSize = sizeReq;
  memcpy(packet.sig, opaq_start + sigOffset, ECC_SIGNATURE_SIZE);
  packet.status = PacketStatus::kWaitVerStart;
  return true;
}

/**
 * Find an empty slot and send it for singing.
 * The signing doesn't occur instantly. Instead, GameController waits until ECC
 * is ready and queues the job.
 */
bool SignedPacketService::SignAndSendData(packet_type packetType,
                                          const uint8_t *data, size_t size) {
  // TODO: include ttl and packet type
  size_t packetId;
  if (!FindPacketOfState(sig_packet_queue_, PacketStatus::kFree, packetId))
    return false;

  SignedPacket &packet = sig_packet_queue_[packetId];
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
  SignedPacket &packet = sig_packet_queue_[signingPacketId];
  signature->toBuffer(packet.sig);
  packet.status = PacketStatus::kWaitTransmit;
}

void SignedPacketService::OnPacketVerFinish(void *isValid) {
  SignedPacket &packet = ver_packet_queue_[verifyingPacketId];
  if (isValid)
    packet.status = PacketStatus::kWaitReceive;
  else
    packet.status = PacketStatus::kFree;
}

void SignedPacketService::ReceivePacket(SignedPacket &packet) {
  switch (packet.type) {
    case packet_type::kScoreAnnounce:
      show_name_app.SetScore(*reinterpret_cast<uint32_t *>(
          packet.data + offsetof(hitcon::ir::ScoreAnnouncePacket, score)));
      break;
    case packet_type::kRestorePet:
      hitcon::app::tama::tama_app.OnRestorePacket(
          reinterpret_cast<hitcon::ir::RestorePetPacket *>(&packet.data));
      break;
    default:
      my_assert(false);
  }
}

void SignedPacketService::VerRoutineFunc() {
  size_t packetId;
  if (FindPacketOfState(ver_packet_queue_, PacketStatus::kWaitVerStart,
                        packetId)) {
    SignedPacket &packet = ver_packet_queue_[packetId];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    bool ret = hitcon::ecc::g_ec_logic.StartVerify(
        packet.data, packet.dataSize, packet.sig,
        (callback_t)&SignedPacketService::OnPacketVerFinish, this);
#pragma GCC diagnostic pop
    if (ret) {
      packet.status = PacketStatus::kWaitVerDone;
      verifyingPacketId = packetId;
    }
  }

  if (FindPacketOfState(ver_packet_queue_, PacketStatus::kWaitReceive,
                        packetId)) {
    ReceivePacket(ver_packet_queue_[packetId]);
    ver_packet_queue_[packetId].status = PacketStatus::kFree;
  }
}

void SignedPacketService::SigRoutineFunc() {
  size_t packetId;
  if (FindPacketOfState(sig_packet_queue_, PacketStatus::kWaitSignStart,
                        packetId)) {
    SignedPacket &packet = sig_packet_queue_[packetId];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    bool ret = hitcon::ecc::g_ec_logic.StartSign(
        packet.data, packet.dataSize,
        (callback_t)&SignedPacketService::OnPacketSignFinish, this);
#pragma GCC diagnostic pop
    if (ret) {
      packet.status = PacketStatus::kWaitSignDone;
      signingPacketId = packetId;
    }
  }

  // Find signed packets and transmit if possible
  if (FindPacketOfState(sig_packet_queue_, PacketStatus::kWaitTransmit,
                        packetId)) {
    SignedPacket &packet = sig_packet_queue_[packetId];
    hitcon::ir::IrData irdata = {.ttl = 0, .type = packet.type};
    memcpy(&irdata.opaq, packet.data, packet.dataSize);
    memcpy(reinterpret_cast<uint8_t *>(&irdata.opaq) + packet.signatureOffset,
           packet.sig, sizeof(packet.sig));
    bool ret = hitcon::ir::irController.SendPacketWithRetransmit(
        reinterpret_cast<uint8_t *>(&irdata),
        packet.dataSize + ECC_SIGNATURE_SIZE + ir::IR_DATA_HEADER_SIZE, 3,
        ::hitcon::ir::AckTag::ACK_TAG_NONE);
    if (ret) packet.status = PacketStatus::kFree;
  }
}

}  // namespace hitcon
