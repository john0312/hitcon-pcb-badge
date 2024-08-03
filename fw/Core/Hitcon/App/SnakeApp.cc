#include <App/SnakeApp.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>

using namespace hitcon::service::sched;

namespace hitcon {
SnakeApp snake_app;

SnakeApp::SnakeApp()
    : _routine_task(30, (task_callback_t)&SnakeApp::Routine, (void*)this,
                    INTERVAL) {}

/* TODO:
 * 1. queue task only once
 * 2. random snake begin
 * 3. event when _len = 128
 * 4. store score
 * 5. PvP and Timer
 */
void SnakeApp::OnEntry() {
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  _direction = DIRECTION_RIGHT;
  _last_direction = DIRECTION_RIGHT;
  _len = 2;
  _body[0] = 36;
  _body[1] = 35;
  _game_over = false;
  GenerateFood();
}

void SnakeApp::OnExit() { scheduler.DisablePeriodic(&_routine_task); }

void SnakeApp::OnButton(button_t button) {
  direction_t btn_direction = NONE;

  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_RIGHT:
      if (_last_direction != DIRECTION_LEFT) btn_direction = DIRECTION_RIGHT;
      break;
    case BUTTON_LEFT:
      if (_last_direction != DIRECTION_RIGHT) btn_direction = DIRECTION_LEFT;
      break;
    case BUTTON_DOWN:
      if (_last_direction != DIRECTION_UP) btn_direction = DIRECTION_DOWN;
      break;
    case BUTTON_UP:
      if (_last_direction != DIRECTION_DOWN) btn_direction = DIRECTION_UP;
      break;
  }
  if (!btn_direction) return;
  _direction = btn_direction;
}

bool SnakeApp::OnSnake(uint8_t index) {
  bool on_snake = false;

  for (uint8_t i = 0; i < _len; i++) {
    if (_body[i] == index) {
      on_snake = true;
      break;
    }
  }
  return on_snake;
}

void SnakeApp::GenerateFood() {
  bool on_snake = true;
  uint8_t index;

  while (on_snake) {
    index = g_fast_random_pool.GetRandom() % (DISPLAY_HEIGHT * DISPLAY_WIDTH);
    on_snake = OnSnake(index);
  }

  _food_index = index;
}

void SnakeApp::Routine(void* unused) {
  if (_game_over) return;

  uint8_t new_head = _body[0];
  uint8_t frame_buf[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {0};
  frame_buf[_food_index] = 1;

  switch (_direction) {
    case DIRECTION_RIGHT:
      if (_body[0] % DISPLAY_WIDTH == DISPLAY_WIDTH - 1)
        _game_over = true;
      else
        new_head++;
      break;
    case DIRECTION_LEFT:
      if (_body[0] % DISPLAY_WIDTH == 0)
        _game_over = true;
      else
        new_head--;
      break;
    case DIRECTION_UP:
      new_head -= DISPLAY_WIDTH;
      break;
    case DIRECTION_DOWN:
      new_head += DISPLAY_WIDTH;
      break;
    default:
      break;
  }
  if (new_head > 127) _game_over = true;
  if (OnSnake(new_head)) _game_over = true;

  if (_game_over) {
    display_set_mode_scroll_text("Game Over");
    return;
  }

  if (_food_index == new_head) {
    _len++;
    GenerateFood();
  }

  for (int8_t i = _len - 1; i > 0; i--) _body[i] = _body[i - 1];

  _body[0] = new_head;

  for (uint8_t i = 0; i < _len; i++) frame_buf[_body[i]] = 1;

  _last_direction = _direction;
  display_set_mode_fixed(frame_buf);
}
}  // namespace hitcon
