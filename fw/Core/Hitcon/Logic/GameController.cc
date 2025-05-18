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

void GameController::OnPrivKeyHashFinish(void* arg2) {
  uint8_t* ptr = reinterpret_cast<uint8_t*>(arg2);
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
  memcpy(irdata.pub_announce.pubkey, pubkey, ECC_PUBKEY_SIZE);

  // TODO: Add proper signature from Certificate Authority
  // For now, initialize with zeros or some placeholder
  static_assert(ECC_SIGNATURE_SIZE == PerBoardData::kPubKeyCertSize);
  memcpy(irdata.pub_announce.sig, g_per_board_data.GetPubKeyCert(),
         ECC_SIGNATURE_SIZE);

  // Calculate the total size of the packet
  uint8_t irdata_len =
      hitcon::ir::IR_DATA_HEADER_SIZE + sizeof(hitcon::ir::PubAnnouncePacket);

  hitcon::ir::irController.SendPacketWithRetransmit(
      reinterpret_cast<uint8_t*>(&irdata), irdata_len, 3);
  return true;
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
}

}  // namespace game
}  // namespace hitcon
