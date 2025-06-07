#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Logic/GameParam.h>
#include <Logic/IrController.h>
#include <Service/HashService.h>
#include <Service/PerBoardData.h>

#include <cstring>

namespace hitcon {

hitcon::game::GameController g_game_controller;

namespace game {

GameController::GameController()
    : state_(0),
      routine_task(960, (callback_t)&GameController::RoutineFunc, this, 490) {}

void GameController::Init() {
  state_ = 1;
  hitcon::service::sched::scheduler.Queue(&routine_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routine_task);
}

void GameController::OnPrivKeyHashFinish(void *arg2) {
  uint8_t *ptr = reinterpret_cast<uint8_t *>(arg2);
  uint64_t privkey;
  memcpy(&privkey, ptr, sizeof(uint64_t));
  hitcon::ecc::g_ec_logic.SetPrivateKey(privkey);
  state_ = 3;
}

bool GameController::TrySendPubAnnounce() {
  // Create a PubAnnouncePacket
  hitcon::ir::IrData irdata = {
      .ttl = 0,
      .type = packet_type::kPubAnnounce,
  };

  // Get the public key
  uint8_t pubkey[ECC_PUBKEY_SIZE];
  bool success = hitcon::ecc::g_ec_logic.GetPublicKey(pubkey);

  if (!success) {
    // Public key not ready yet, will try again later
    return false;
  }

  // Copy the public key to the packet
  memcpy(irdata.opaq.pub_announce.pubkey, pubkey, ECC_PUBKEY_SIZE);

  // TODO: Add proper signature from Certificate Authority
  // For now, initialize with zeros or some placeholder
  static_assert(ECC_SIGNATURE_SIZE == PerBoardData::kPubKeyCertSize);
  memcpy(irdata.opaq.pub_announce.sig, g_per_board_data.GetPubKeyCert(),
         ECC_SIGNATURE_SIZE);

  // Calculate the total size of the packet
  uint8_t irdata_len =
      hitcon::ir::IR_DATA_HEADER_SIZE + sizeof(hitcon::ir::PubAnnouncePacket);

  return hitcon::ir::irController.SendPacketWithRetransmit(
      reinterpret_cast<uint8_t *>(&irdata), irdata_len, 3);
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
bool GameController::SignAndSendData(packet_type packetType,
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

void GameController::OnPacketSignFinish(hitcon::ecc::Signature *signature) {
  SignedPacket &packet = packet_queue_[signingPacketId];
  signature->toBuffer(packet.sig);
  packet.status = PacketStatus::kWaitTransmit;
}

void GameController::RoutineFunc() {
  if (state_ == 1) {
    memcpy(&privkey_src_[0], PRIVATE_KEY_SRC_PREFIX,
           sizeof(PRIVATE_KEY_SRC_PREFIX));
    memcpy(&privkey_src_[sizeof(PRIVATE_KEY_SRC_PREFIX)],
           g_per_board_data.GetPerBoardSecret(), PerBoardData::kSecretLen);
    bool ret = hitcon::hash::g_hash_service.StartHash(
        privkey_src_, sizeof(PRIVATE_KEY_SRC_PREFIX) + PerBoardData::kSecretLen,
        (service::sched::task_callback_t)&GameController::OnPrivKeyHashFinish,
        this);
    if (ret) {
      state_ = 2;
    }
  } else if (state_ == 2) {
    // Waiting for hash processor.
  } else if (state_ == 3) {
    // TODO: Publish the public key through PubAnnouncePacket.
    bool ret = TrySendPubAnnounce();
    if (ret) state_ = 4;
  } else if (state_ == 4) {
    // Scan the packet queue and sign them if possible
    for (size_t packetId = 0; packetId < PACKET_QUEUE_SIZE; ++packetId) {
      if (packet_queue_[packetId].status != kWaitSignStart) continue;
      SignedPacket &packet = packet_queue_[packetId];
      bool ret = hitcon::ecc::g_ec_logic.StartSign(
          packet.data, packet.dataSize,
          (callback_t)&GameController::OnPacketSignFinish, this);
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
          packet.dataSize + ECC_SIGNATURE_SIZE, 3);
      packet.status = PacketStatus::kFree;
    }
  }
}

}  // namespace game
}  // namespace hitcon
