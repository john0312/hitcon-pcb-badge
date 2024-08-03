#ifndef SNAKE_APP_H
#define SNAKE_APP_H

#include "app.h"
#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

using namespace hitcon::service::sched;


namespace hitcon {

// interval for snake moving
constexpr unsigned INTERVAL = 350;
enum direction_t {
  NONE = 0,
  DIRECTION_RIGHT,
  DIRECTION_LEFT,
  DIRECTION_UP,
  DIRECTION_DOWN,
};
class SnakeApp : public App {
private:
  PeriodicTask _routine_task;
  uint8_t _body[DISPLAY_HEIGHT * DISPLAY_WIDTH]; //0: head
  uint8_t _len;
  direction_t _direction;
  direction_t _last_direction;
  uint8_t _has_food = false;
  uint8_t _food_index;
  bool _game_over;

  void GenerateFood();
  void Routine(void* unused);
  bool OnSnake(uint8_t index);
public:
  SnakeApp();
  virtual ~SnakeApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern SnakeApp snake_app;

}  // namespace hitcon

#endif  // SNAKE_APP_H
