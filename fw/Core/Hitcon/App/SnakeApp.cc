#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <App/ShowScoreApp.h>
#include <App/SnakeApp.h>
#include <Logic/BadgeController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

#include "ShowScoreApp.h"

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::app::snake;

namespace hitcon {
namespace app {
namespace snake {

SnakeApp snake_app;

SnakeApp::SnakeApp()
    : _routine_task(30, (task_callback_t)&SnakeApp::Routine, (void*)this,
                    INTERVAL) {}

/* TODO:
 *  1. (done) queue task only once
 *  2. random snake begin
 *  3. event when _len = 128
 *  4. store score
 *  6. add OnButton when game over
 *  7. (done 1ms) measure Routine time
 *  8. (done) add Ready... scrolling text
 *  9. (done) implement multi-player mode
 * 10. xboard on connect/disconnect event
 * 11. check connection status
 * 12. (done) add menu to choose game mode
 * 13. dynamic interval?
 * 14. show win/lose and score when game over
 */
void SnakeApp::Init() {
  scheduler.Queue(&_routine_task, nullptr);
  mode = MODE_NONE;
}

void SnakeApp::OnEntry() {
  _game_over = false;
  if (mode == MODE_NONE)  // default game mode is single player
    mode = MODE_SINGLEPLAYER;
  else if (mode == MODE_MULTIPLAYER) {
    g_xboard_logic.SetOnPacketArrive((callback_t)&SnakeApp::OnXBoardRecv, this,
                                     SNAKE_RECV_ID);
  }
  _state = STATE_WAIT;
  display_set_mode_scroll_text("Ready...");
}

void SetSingleplayer() { snake_app.mode = MODE_SINGLEPLAYER; }

void SetMultiplayer() { snake_app.mode = MODE_MULTIPLAYER; }

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
    case BUTTON_OK:
      if (_game_over) badge_controller.change_app(this);
      if (_state == STATE_WAIT) {
        _state = STATE_PLAYING;
        if (mode == MODE_MULTIPLAYER) {
          uint8_t code = PACKET_GAME_START;
          g_xboard_logic.QueueDataForTx(&code, 1, SNAKE_RECV_ID);
        }

        InitGame();
      }
      break;
    case BUTTON_BACK:
      badge_controller.change_app(&main_menu);
      break;
    case BUTTON_LONG_BACK:
      badge_controller.change_app(&show_name_app);
      break;
    default:
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

void SnakeApp::OnXBoardRecv(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  switch (packet->data[0]) {
    case PACKET_GAME_OVER:
      _game_over = true;
      display_set_mode_scroll_text("Game Over");
      break;
    case PACKET_GET_FOOD:
      _len++;
      break;
    case PACKET_GAME_START:
      if (_state == STATE_WAIT) {
        _state = STATE_PLAYING;
        InitGame();
      }
      break;
  }
}

void SnakeApp::InitGame() {
  scheduler.EnablePeriodic(&_routine_task);
  _direction = DIRECTION_RIGHT;
  _last_direction = DIRECTION_RIGHT;
  _len = 2;
  _body[0] = 36;
  _body[1] = 35;
  _score = 0;
  GenerateFood();
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
  if (_game_over) return;  // disable perioid task

  uint8_t new_head = _body[0];
  uint8_t frame_buf[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {0};

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
    show_score_app.SetScore(_score);
    if (mode == MODE_MULTIPLAYER) {
      uint8_t code = PACKET_GAME_OVER;
      g_xboard_logic.QueueDataForTx(&code, 1, SNAKE_RECV_ID);
    }
    badge_controller.change_app(&show_score_app);
    return;
  }

  if (_food_index == new_head) {
    _food_index = -1;
    if (mode == MODE_SINGLEPLAYER)
      _len++;
    else {
      uint8_t code = PACKET_GET_FOOD;
      g_xboard_logic.QueueDataForTx(&code, 1, SNAKE_RECV_ID);
    }
    _score++;
  }

  // shift snake body
  for (int8_t i = _len - 1; i > 0; i--) _body[i] = _body[i - 1];
  _body[0] = new_head;

  for (uint8_t i = 0; i < _len; i++) frame_buf[_body[i]] = 1;
  if (_food_index == -1) GenerateFood();

  _last_direction = _direction;
  frame_buf[_food_index] = 1;
  display_set_mode_fixed(frame_buf);
}
}  // namespace snake
}  // namespace app
}  // namespace hitcon
