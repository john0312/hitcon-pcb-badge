#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Logic/GameParam.h>
#include <Logic/IrController.h>
#include <Logic/NvStorage.h>
#include <Service/HashService.h>
#include <Service/PerBoardData.h>
#include <Service/SignedPacketService.h>

#include <cstring>

using hitcon::ir::IR_USERNAME_LEN;
namespace hitcon {

hitcon::game::GameController g_game_controller;

namespace game {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
GameController::GameController()
    : state_(0), pubAnnounceCnt(kPubAnnounceCycleInterval - 2),
      pubAnnounceTask(960, (callback_t)&GameController::TrySendPubAnnounce,
                      this, 1490) {}
#pragma GCC diagnostic pop

void GameController::Init() {
  hitcon::ecc::g_ec_logic.SetPrivateKey(g_per_board_data.GetPrivKey());
  hitcon::service::sched::scheduler.Queue(&pubAnnounceTask, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&pubAnnounceTask);
}

bool GameController::SendTwoBadgeActivity(const TwoBadgeActivity &data) {
  hitcon::ir::TwoBadgeActivityPacket packet;
  if (!SetBufferToUsername(packet.user1)) {
    // Not ready.
    return false;
  }
  memcpy(packet.user2, data.otherUser, IR_USERNAME_LEN);
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
  if (!SetBufferToUsername(packet.user)) {
    // Not ready.
    return false;
  }
  packet.power = data.power;
  static_assert(sizeof(packet.nonce) == sizeof(data.nonce));
  memcpy(packet.nonce, &data.nonce, sizeof(packet.nonce));
  return g_signed_packet_service.SignAndSendData(
      packet_type::kProximity, reinterpret_cast<uint8_t *>(&packet),
      sizeof(packet) - ECC_SIGNATURE_SIZE);
}

bool GameController::SendSingleBadgeActivity(const SingleBadgeActivity &data) {
  hitcon::ir::SingleBadgeActivityPacket packet;
  if (!SetBufferToUsername(packet.user)) {
    // Not ready.
    return false;
  }
  uint16_t myScore = data.myScore;
  if (myScore >= 1024) {
    myScore = 1023;
  }
  packet.event_type = data.eventType;
  packet.event_data[0] = (myScore & 0xFF);
  packet.event_data[1] = (myScore & 0x0300) >> 8;
  packet.event_data[1] |= (data.nonce & 0x03F) << 2;
  packet.event_data[2] = (data.nonce & 0x3FC0) >> 6;
  return g_signed_packet_service.SignAndSendData(
      packet_type::kSingleBadgeActivity, reinterpret_cast<uint8_t *>(&packet),
      sizeof(packet) - ECC_SIGNATURE_SIZE);
}

void GameController::NotifyPubkeyAck() {
  g_nv_storage.GetCurrentStorage().game_storage.user_id_acknowledged = 1;
  g_nv_storage.MarkDirty();
  hitcon::service::sched::scheduler.DisablePeriodic(&pubAnnounceTask);
}

const uint8_t *GameController::GetUsername() {
  // To guarantee maximum entropy, we use the last 4 bytes of the x-coordinate
  // of the public key, which is byte 3 - 6 inclusive.
  const uint8_t *ptr = hitcon::ecc::g_ec_logic.GetPublicKey();
  if (!ptr) return ptr;
  ptr += (ECC_PUBKEY_SIZE - IR_USERNAME_LEN - 1);
  return ptr;
}

bool GameController::SetBufferToUsername(uint8_t *ptr) {
  auto *username = GetUsername();
  if (!username) {
    // Not ready.
    memset(ptr, 0, IR_USERNAME_LEN);
    return false;
  }
  memcpy(ptr, username, IR_USERNAME_LEN);
  return true;
}
void GameController::TrySendPubAnnounce() {
  pubAnnounceCnt++;
  if (pubAnnounceCnt >= kPubAnnounceCycleInterval) {
    pubAnnounceCnt -= kPubAnnounceCycleInterval;
  } else {
    return;
  }

  // Create a PubAnnouncePacket
  hitcon::ir::IrData irdata = {
      .ttl = 0,
      .type = packet_type::kPubAnnounce,
  };

  // Get the public key
  const uint8_t *pubkey = hitcon::ecc::g_ec_logic.GetPublicKey();

  if (!pubkey) {
    // Public key not ready yet, will try again later
    return;
  }

  // Copy the public key to the packet
  memcpy(irdata.opaq.pub_announce.pubkey, pubkey, ECC_PUBKEY_SIZE);

  static_assert(ECC_SIGNATURE_SIZE == PerBoardData::kPubKeyCertSize);
  memcpy(irdata.opaq.pub_announce.sig, g_per_board_data.GetPubKeyCert(),
         ECC_SIGNATURE_SIZE);

  // Calculate the total size of the packet
  uint8_t irdata_len =
      hitcon::ir::IR_DATA_HEADER_SIZE + sizeof(hitcon::ir::PubAnnouncePacket);

  hitcon::ir::irController.SendPacketWithRetransmit(
      reinterpret_cast<uint8_t *>(&irdata), irdata_len, 3,
      ::hitcon::ir::AckTag::ACK_TAG_PUBKEY_RECOG);
}

}  // namespace game
}  // namespace hitcon
