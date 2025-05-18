#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Logic/GameParam.h>
#include <Service/HashService.h>
#include <Service/PerBoardData.h>

#include <cstring>

namespace hitcon {

hitcon::game::GameController g_game_controller;

namespace game {

GameController::GameController() : state_(0) {}

void GameController::Init() { state_ = 1; }

void GameController::OnPrivKeyHashFinish(void* arg2) {
  uint8_t* ptr = reinterpret_cast<uint8_t*>(arg2);
  uint64_t privkey;
  memcpy(&privkey, ptr, sizeof(uint64_t));
  hitcon::ecc::g_ec_logic.SetPrivateKey(privkey);
  state_ = 3;
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
  }
}

}  // namespace game
}  // namespace hitcon
