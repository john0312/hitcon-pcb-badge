#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <App/ShowScoreApp.h>
#include <App/SpaceshipApp.h>
#include <Logic/BadgeController.h>
#include <Logic/GameScore.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

using namespace hitcon::service::sched;
using hitcon::game::EventType;

namespace hitcon {
namespace app {
namespace spaceship {

SpaceshipApp spaceship_app;

SpaceshipApp::SpaceshipApp()
    : _routine_task(30, (task_callback_t)&SpaceshipApp::Routine, (void*)this,
                    INTERVAL) {}

// Init: put routine_task into queue
void SpaceshipApp::Init() { scheduler.Queue(&_routine_task, nullptr); }

// 1. OnEntry: menu -> ready
void SpaceshipApp::OnEntry() {
  _state = INIT;
  display_set_mode_scroll_text("Ready...");
}
// 2. OnExit
void SpaceshipApp::OnExit() {
  scheduler.DisablePeriodic(&_routine_task);
  _state = END;
}
// 3. OnButton
void SpaceshipApp::OnButton(button_t button) {}
// 4. OnEdgeButton
void SpaceshipApp::OnEdgeButton(button_t button) {
  if ((button & BUTTON_VALUE_MASK) == BUTTON_BACK) {
    badge_controller.BackToMenu(this);
    return;
  }

  if (_state == RUN) {
    switch (button & BUTTON_VALUE_MASK) {
      case BUTTON_DOWN:  // move _my_position down
        if (_my_position < PLANE_UPPER_BOUND) {
          _SyncMyPlane(_my_position << 1);
        }
        break;
      case BUTTON_UP:  // move _my_position up
        if (_my_position > PLANE_LOWER_BOUND) {
          _SyncMyPlane(_my_position >> 1);
        }
        break;
      case BUTTON_RIGHT:  // emit bullet
        _bullets[2] |= _my_position;
        break;
      default:
        break;
    }
    return;
  }

  if ((_state == INIT) & ((button & BUTTON_VALUE_MASK) == BUTTON_OK)) {
    _StartGame();
    return;
  }
}

void SpaceshipApp::_StartGame() {
  // setup my plane
  _my_plane[2] = {0};
  _my_position = 0;
  _SyncMyPlane(PLANE_LOWER_BOUND);
  // setup enemy
  _enemy_position[DISPLAY_WIDTH] = {0};
  _GenerateEnemy();
  // setup score
  _score = 0;
  // setup bullets
  _bullets[DISPLAY_WIDTH] = {0};
  // setup display
  _frame_buf[DISPLAY_WIDTH] = {0};
  // setup state
  _state = RUN;
  scheduler.EnablePeriodic(&_routine_task);
}

// for callback
void SpaceshipApp::Routine(void* unused) {
  // check current status
  // ## 1. check collision
  _CheckCollision();
  if (_state == GAME_OVER) {
    show_score_app.SetScore(_score);
    g_game_score.MarkScore(GameScoreType::GAME_SPACESHIP, _score);
    badge_controller.change_app(&show_score_app);
    return;
  }
  // ## 2. update score & bullets
  _UpdateScoreBullets();

  // render current status
  _Render();
  // move on
  // ## 1. move all bullets to right
  _MoveBulletsToRight();
  // ## 2. generate enemy or move enemy to left
  _MoveEnemyToLeft();
  if (!_has_enemy) _GenerateEnemy();
}

void SpaceshipApp::_SyncMyPlane(uint8_t new_position) {
  if (new_position != _my_position) {
    _my_position = new_position;
    _my_plane[0] = _my_position | _my_position << 1 | _my_position >> 1;
    _my_plane[1] = _my_position;
  }
}

void SpaceshipApp::_Render() {
  for (uint8_t i = 0; i < DISPLAY_WIDTH; i++) {
    _frame_buf[i] = _enemy_position[i] | _bullets[i];
    if (i < 2) {
      _frame_buf[i] |= _my_plane[i];
    }
  }
  display_set_mode_fixed_packed(_frame_buf);
}

void SpaceshipApp::_CheckCollision() {
  for (uint8_t i = 0; i < 2; i++) {
    if ((_my_plane[i] & _enemy_position[i]) > 0) {
      _state = GAME_OVER;  // game over!
      break;
    }
  }
}

void SpaceshipApp::_UpdateScoreBullets() {
  for (uint8_t i = 0; i < DISPLAY_WIDTH; i++) {
    if ((_bullets[i] & _enemy_position[i]) > 0) {
      _score += 1;
      _bullets[i] &= ~_enemy_position[i];  // remove bullet
      _enemy_position[i] = 0;              // remove enemy
      _has_enemy = false;
      break;
    }
  }
}

void SpaceshipApp::_MoveBulletsToRight() {
  for (uint8_t i = DISPLAY_WIDTH - 1; i >= 2; i--) {
    _bullets[i] = _bullets[i - 1];
  }
}

void SpaceshipApp::_GenerateEnemy() {
  uint8_t row = g_fast_random_pool.GetRandom() % (DISPLAY_HEIGHT - 2);
  _enemy_position[DISPLAY_WIDTH - 1] = 1 << (row + 1);
  _has_enemy = true;
}

void SpaceshipApp::_MoveEnemyToLeft() {
  if (!_has_enemy) return;
  _has_enemy = false;
  for (uint8_t i = 0; i < (DISPLAY_WIDTH - 1); i++) {
    _enemy_position[i] = _enemy_position[i + 1];
    _enemy_position[i + 1] = 0;
    if (_enemy_position[i] > 0) {
      _has_enemy = true;
    }
  }
}

}  // namespace spaceship
}  // namespace app
}  // namespace hitcon
