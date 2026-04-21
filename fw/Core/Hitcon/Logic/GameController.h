#ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
#define HITCON_LOGIC_GAME_CONTROLLER_H_

#include <Logic/BadgeId.h>
#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

namespace hitcon {
namespace game {

enum EventType : uint8_t {
  kNone = 0,
  kSnake = 1,
  kTetris = 2,
  kDino = 3,
  kTama = 4,
  kSpaceship = 5,
  kShake = 16
};

class GameController {
 public:
  GameController();

  void Init();
};

}  // namespace game

extern hitcon::game::GameController g_game_controller;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
