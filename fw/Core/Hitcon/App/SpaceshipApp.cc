#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <App/ShowScoreApp.h>
#include <App/SpaceshipApp.h>
#include <Logic/BadgeController.h>
#include <Logic/GameScore.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

#include <algorithm>

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
  if (_routine_task.IsEnabled()) {
    scheduler.DisablePeriodic(&_routine_task);
  }
  _state = END;
}
// 3. OnButton
void SpaceshipApp::OnButton(button_t button) {}
// 4. OnEdgeButton
void SpaceshipApp::OnEdgeButton(button_t button) {
  constexpr int last_row = DISPLAY_WIDTH * (DISPLAY_HEIGHT - 2);

  if ((button & BUTTON_VALUE_MASK) == BUTTON_BACK) {
    badge_controller.BackToMenu(this);
    return;
  }

  if (_state == RUN) {
    switch (button & BUTTON_VALUE_MASK) {
      case BUTTON_DOWN:  // move _my_position down
        if (_my_position < last_row) _SyncMyPlane(_my_position + DISPLAY_WIDTH);
        break;
      case BUTTON_UP:  // move _my_position up
        if (_my_position > DISPLAY_WIDTH)
          _SyncMyPlane(_my_position - DISPLAY_WIDTH);
        break;
      case BUTTON_RIGHT:  // emit bullet
        _bullets[_num_bullets] = _my_position + 2;
        _num_bullets += 1;
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
  _SyncMyPlane(DISPLAY_WIDTH);
  // setup enemy
  _GenerateEnemy();
  _has_enemy = true;
  // setup bullets
  _num_bullets = 0;
  // setup score
  _score = 0;
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
  if (_has_enemy) {
    _enemy_position -= 1;
  } else {
    _GenerateEnemy();
    _has_enemy = true;
  }
}

void SpaceshipApp::_SyncMyPlane(uint8_t new_position) {
  if (new_position != _my_position) {
    _my_position = new_position;
    _my_plane[0] = _my_position;
    _my_plane[1] = _my_position + 1;
    _my_plane[2] = _my_position - DISPLAY_WIDTH;
    _my_plane[3] = _my_position + DISPLAY_WIDTH;
  }
}

void SpaceshipApp::_Render() {
  uint8_t frame_buf[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {0};
  // ## 1. render plane
  for (uint8_t i = 0; i < 4; i++) {
    frame_buf[_my_plane[i]] = 1;
  }
  // ## 2. render enemy
  frame_buf[_enemy_position] = 1;
  // ## 3. render bullets
  for (uint8_t i = 0; i < _num_bullets; i++) {
    frame_buf[_bullets[i]] = 1;
  }
  display_set_mode_fixed(frame_buf);
}

void SpaceshipApp::_CheckCollision() {
  for (uint8_t i = 0; i < 4; i++) {
    if (_my_plane[i] == _enemy_position) {
      _state = GAME_OVER;  // game over!
      break;
    }
  }
}

void SpaceshipApp::_UpdateScoreBullets() {
  uint8_t left_bullets = 0;
  for (uint8_t i = 0; i < _num_bullets; i++) {
    if (_bullets[i] == _enemy_position) {
      _score += 1;
      _has_enemy = false;
      // this bullet is used.
    } else {
      _bullets[left_bullets] = _bullets[i];
      left_bullets += 1;
    }
  }
  _num_bullets = left_bullets;  // update _num_bullets
}

void SpaceshipApp::_MoveBulletsToRight() {
  uint8_t left_bullets = 0;
  for (uint8_t i = 0; i < _num_bullets; i++) {
    _bullets[i] += 1;
    if (_bullets[i] % DISPLAY_WIDTH > 0) {  // check bullet within region
      _bullets[left_bullets] = _bullets[i];
      left_bullets += 1;
    }
  }
  _num_bullets = left_bullets;  // update _num_bullets
}

void SpaceshipApp::_GenerateEnemy() {
  uint8_t row = g_fast_random_pool.GetRandom() % (DISPLAY_HEIGHT - 2);
  _enemy_position = (row + 2) * DISPLAY_WIDTH - 1;
}

}  // namespace spaceship
}  // namespace app
}  // namespace hitcon
