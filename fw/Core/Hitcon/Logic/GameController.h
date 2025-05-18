#ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
#define HITCON_LOGIC_GAME_CONTROLLER_H_

#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

namespace hitcon {
namespace game {

constexpr uint8_t PRIVATE_KEY_SRC_PREFIX[] = {'2', '0', '2', '5',
                                              'P', 'R', 'I', 'V'};

class GameController {
 public:
  GameController();

  void Init();

 private:
  /*
  Internal state of the game controller.
  0 - Not initialized.
  1 - Initialized. After Init().
  2 - Waiting for hash processor to compute private key.
  3 - Got private key.
  */
  int state_;

  uint8_t
      privkey_src_[PerBoardData::kSecretLen + sizeof(PRIVATE_KEY_SRC_PREFIX)];

  hitcon::service::sched::PeriodicTask routine_task;

  bool TrySendPubAnnounce();
  void OnPrivKeyHashFinish(void* arg2);
  void RoutineFunc();
};

}  // namespace game

extern hitcon::game::GameController g_game_controller;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
