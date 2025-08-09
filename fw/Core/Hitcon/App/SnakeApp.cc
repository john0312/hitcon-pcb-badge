#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <App/ShowScoreApp.h>
#include <App/SnakeApp.h>
#include <Logic/BadgeController.h>
#include <Logic/GameScore.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::app::snake;
using hitcon::app::multiplayer::MultiplayerGame;
using hitcon::app::multiplayer::PlayerCount;
using hitcon::game::EventType;

namespace hitcon {
namespace app {
namespace snake {

SnakeApp snake_app;

SnakeApp::SnakeApp()
    : _routine_task(30, (task_callback_t)&SnakeApp::Routine, (void*)this,
                    INTERVAL) {}

/* TODO:
 *  3. event when _len = 128
 * 13. dynamic interval?
 * 14. show win/lose and score when game over
 */
void SnakeApp::Init() {
  scheduler.Queue(&_routine_task, nullptr);
  mode = MODE_NONE;
}

void SnakeApp::GameEntry() {
  _game_over = false;
  _state = STATE_WAIT;
  display_set_mode_scroll_text("Ready...");
}

void SetSingleplayer() { snake_app.SetPlayerCount(PlayerCount::SINGLEPLAYER); }

void SetMultiplayer() { snake_app.SetPlayerCount(PlayerCount::MULTIPLAYER); }

void SnakeApp::GameExit() { scheduler.DisablePeriodic(&_routine_task); }

void SnakeApp::StartGame() {
  _state = STATE_PLAYING;
  InitGame();
}

void SnakeApp::AbortGame() { badge_controller.BackToMenu(this); }

void SnakeApp::GameOver() {
  _game_over = true;
  // game over screen
  show_score_app.SetScore(GetScore());
  g_game_score.MarkScore(GameScoreType::GAME_SNAKE, GetScore());
  badge_controller.change_app(&show_score_app);
}

void SnakeApp::RecvAttackPacket(PacketCallbackArg* packet) { _len++; }

RecvFnId SnakeApp::GetXboardRecvId() const { return SNAKE_RECV_ID; }

EventType SnakeApp::GetGameType() const { return EventType::kSnake; }

uint32_t SnakeApp::GetScore() const { return _score; }

void SnakeApp::OnEdgeButton(button_t button) {
  direction_t btn_direction = NONE;
  if (button & BUTTON_KEYUP_BIT) return;

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
        if (IsMultiplayer()) {
          SendStartGame();
        }
        StartGame();
      }
      break;
    case BUTTON_BACK:
      if (IsMultiplayer()) {
        SendAbortGame();
      }
      AbortGame();
      break;
    default:
      break;
  }
  if (!btn_direction) return;
  _direction = btn_direction;
}

void SnakeApp::OnButton(button_t button) {}

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

void SnakeApp::InitGame() {
  scheduler.EnablePeriodic(&_routine_task);
  _direction = DIRECTION_RIGHT;
  _last_direction = DIRECTION_RIGHT;
  _len = 2;
  uint8_t row = g_fast_random_pool.GetRandom() % (DISPLAY_HEIGHT - 4) + 2;
  uint8_t col = g_fast_random_pool.GetRandom() % (DISPLAY_WIDTH - 4) + 1;
  _body[0] = row * DISPLAY_WIDTH + col;
  _body[1] = _body[0] - 1;
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
    // local game over
    if (IsMultiplayer()) {
      SendGameOver();
    } else {
      UploadSingleplayerScore();
    }
    GameOver();
    return;
  }

  if (_food_index == new_head) {
    _food_index = -1;
    if (!IsMultiplayer())
      _len++;
    else {
      SendAttack(1);
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
