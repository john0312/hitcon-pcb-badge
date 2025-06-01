#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Logic/GameParam.h>
#include <Logic/IrController.h>
#include <Logic/NvStorage.h>
#include <Service/HashService.h>
#include <Service/PerBoardData.h>
#include <Service/SignedPacketService.h>

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

bool GameController::SendTwoBadgeActivity(const TwoBadgeActivity &data) {
  hitcon::ir::TwoBadgeActivityPacket packet;
  GetUsername(packet.user1);
  memcpy(packet.user2, data.otherUser, hitcon::ir::IR_USERNAME_LEN);
  // Game Type: byte 0 bits 0:4
  packet.game_data[0] = data.gameType & 0xf;
  // Player 1 Score: byte 0 bits 4:8, byte 1 bits 0:6
  packet.game_data[0] |= (data.myScore & 0xf) << 4;
  packet.game_data[1] = (data.myScore & 0x3f0) >> 4;
  // Player 2 Score: byte 1 bits 6:8, byte 2 bits 0:8
  packet.game_data[1] |= (data.otherScore & 0x3) << 6;
  packet.game_data[2] = (data.otherScore & 0x3fc) >> 2;
  // Nonce: byte 3, byte 4
  *reinterpret_cast<uint16_t *>(&packet.game_data[3]) = data.nonce;
  return g_signed_packet_service.SignAndSendData(
      packet_type::kTwoBadgeActivity, reinterpret_cast<uint8_t *>(&packet),
      sizeof(packet) - ECC_SIGNATURE_SIZE);
}

bool GameController::SendProximity(const Proximity &data) {
  hitcon::ir::ProximityPacket packet;
  GetUsername(packet.user);
  packet.power = data.power;
  static_assert(sizeof(packet.nonce) == sizeof(data.nonce));
  memcpy(packet.nonce, &data.nonce, sizeof(packet.nonce));
  return g_signed_packet_service.SignAndSendData(
      packet_type::kProximity, reinterpret_cast<uint8_t *>(&packet),
      sizeof(packet) - ECC_SIGNATURE_SIZE);
}

bool GameController::SendSingleBadgeActivity(const SingleBadgeActivity &data) {
  hitcon::ir::SingleBadgeActivityPacket packet;
  GetUsername(packet.user);
  packet.event_type = data.eventType;
  static_assert(sizeof(packet.event_data) == sizeof(data.eventData));
  memcpy(packet.event_data, data.eventData, sizeof(packet.event_data));
  return g_signed_packet_service.SignAndSendData(
      packet_type::kSingleBadgeActivity, reinterpret_cast<uint8_t *>(&packet),
      sizeof(packet) - ECC_SIGNATURE_SIZE);
}

void GameController::NotifyPubkeyAck() {
  if (state_ == 4) {
    g_nv_storage.GetCurrentStorage().game_storage.user_id_acknowledged = 1;
    g_nv_storage.MarkDirty();
    state_ = 5;
  }
}

void GameController::OnPrivKeyHashFinish(void *arg2) {
  uint8_t *ptr = reinterpret_cast<uint8_t *>(arg2);
  uint64_t privkey;
  memcpy(&privkey, ptr, sizeof(uint64_t));
  hitcon::ecc::g_ec_logic.SetPrivateKey(privkey);
  state_ = 3;
}

void GameController::GetUsername(uint8_t *buf) {
  // TODO: We use the first few bytes of pubkey as username for now.
  // If needed, hash the public key to get a more unique identifier.
  uint8_t pubkey[ECC_PUBKEY_SIZE];
  hitcon::ecc::g_ec_logic.GetPublicKey(pubkey);
  memcpy(buf, pubkey, hitcon::ir::IR_USERNAME_LEN);
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
      reinterpret_cast<uint8_t *>(&irdata), irdata_len, 3,
      ::hitcon::ir::AckTag::ACK_TAG_PUBKEY_RECOG);
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
  }
  // TODO: disable the periodic task if it stays idle forever.
}

}  // namespace game
}  // namespace hitcon
