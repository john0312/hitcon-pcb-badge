#ifndef SPACESHIP_APP_H
#define SPACESHIP_APP_H

#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "app.h"

using namespace hitcon::service::sched;

namespace hitcon {
namespace app {
namespace spaceship {

enum state_t { INIT, RUN, GAME_OVER, END };

class SpaceshipApp : public App {
 private:
  static constexpr unsigned INTERVAL = 350;
  static constexpr unsigned PLANE_LOWER_BOUND = 1 << 1;
  static constexpr unsigned PLANE_UPPER_BOUND = 1 << 6;

  PeriodicTask _routine_task;
  uint8_t _my_position;
  uint8_t _my_plane[2];
  uint8_t _enemy_position[DISPLAY_WIDTH];
  bool _has_enemy;
  uint32_t _score;
  uint8_t _state;

  // handle bullets
  uint8_t _bullets[DISPLAY_WIDTH];
  // for display
  uint8_t _frame_buf[DISPLAY_WIDTH];

  void Routine(void* unused);
  void _StartGame();
  void _GenerateEnemy();
  void _Render();
  void _SyncMyPlane(uint8_t new_position);
  void _MoveEnemyToLeft();
  void _MoveBulletsToRight();
  void _CheckCollision();
  void _UpdateScoreBullets();

 public:
  SpaceshipApp();
  virtual ~SpaceshipApp() = default;

  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void OnEdgeButton(button_t button) override;
};

extern SpaceshipApp spaceship_app;

}  // namespace spaceship
}  // namespace app
}  // namespace hitcon

#endif  // SPACESHIP_APP_H
