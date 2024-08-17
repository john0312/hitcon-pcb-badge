#include "DinoApp.h"

#include <App/MainMenuApp.h>
#include <App/ShowScoreApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameScore.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

#include <cmath>

using namespace hitcon::service::sched;

namespace hitcon {
namespace app {
namespace dino {

DinoApp dino_app;

DinoApp::DinoApp()
    : _routine_task(930, (task_callback_t)&DinoApp::Routine, (void*)this,
                    INTERVAL) {}

void DinoApp::Init() { scheduler.Queue(&_routine_task, nullptr); }

void DinoApp::OnEntry() {
  memset(_obstacle_frame, 0, OBSTACAL_FRAME_WIDTH * sizeof(uint8_t));
  _curr_dino_frame = &dino_running_bitmap[0];
  _obstacle_interval = 0;
  _generate_obs_rate = 8;
  _game_over = 0;
  _score = 0;
  _dino_state = DINO_RUN;
  _dino_ani_frame = 0;
  _dino_jump_vel = DINO_INITIAL_VEL;
  scheduler.EnablePeriodic(&_routine_task);
}

void DinoApp::OnExit() { scheduler.DisablePeriodic(&_routine_task); }

void DinoApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
      badge_controller.change_app(&main_menu);
      break;
    case BUTTON_LONG_BACK:
      badge_controller.change_app(&show_name_app);
      break;
    default:
      break;
  }
  return;
}

void DinoApp::OnEdgeButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_DOWN:
      if (button & BUTTON_KEYDOWN_BIT)
        _dino_state = DINO_CRUNCH;
      else if (button & BUTTON_KEYUP_BIT)
        _dino_state = DINO_RUN;
      break;
    case BUTTON_UP:
      _dino_state = DINO_JUMP;
      break;
  }
}

void DinoApp::Routine(void* unused) {
  if (_game_over) {
    _score_show_time++;
    if (_score_show_time == DINO_SHOW_SCORE_TIME)
      badge_controller.change_app(&dino_app);
    return;
  }
  // Shift frame
  for (uint8_t i = 0; i < OBSTACAL_FRAME_WIDTH; i++) {
    _obstacle_frame[i] = _obstacle_frame[i + 1];
  }
  _obstacle_interval++;
  // Generate obstacle
  if (_obstacle_interval > DINO_OBS_LEAST_DISTANCE) {
    if ((_obstacle_interval > DISPLAY_WIDTH * 2) ||
        (g_fast_random_pool.GetRandom() % _generate_obs_rate == 0)) {
      switch (g_fast_random_pool.GetRandom() % 4) {
        case 0:  // bird
          writeObsByte(DISPLAY_WIDTH, bird_bitmap[0]);
          writeObsByte(DISPLAY_WIDTH + 1, bird_bitmap[1]);
          writeObsByte(DISPLAY_WIDTH + 2, bird_bitmap[2]);
          _obstacle_interval = -6;
          break;
        case 1:  // big cactus
          writeObsByte(DISPLAY_WIDTH, big_cactus_bitmap[0]);
          writeObsByte(DISPLAY_WIDTH + 1, big_cactus_bitmap[1]);
          writeObsByte(DISPLAY_WIDTH + 2, big_cactus_bitmap[2]);
          _obstacle_interval = -5;
          break;
        case 2:  // small tall cactus
          writeObsByte(DISPLAY_WIDTH, small_tall_cactus_bitmap[0]);
          _obstacle_interval = -3;
          break;
        case 3:  // small short cactus
          writeObsByte(DISPLAY_WIDTH, small_short_cactus_bitmap[0]);
          _obstacle_interval = -3;
          break;
        default:
          my_assert(false);
      }
    }
  }
  // Determine dinosaur movement
  // Input is only available when dinosaur is on the ground
  if (_dino_jump_vel == DINO_INITIAL_VEL) {
    switch (_dino_state) {
      case DINO_CRUNCH:
        _curr_dino_frame = &dino_crunching_bitmap[_dino_ani_frame];
        break;
      case DINO_JUMP:
        _dino_jump_vel = DINO_HEIGHT - 1;
        _curr_dino_frame = &dino_jumping_bitmap[abs(_dino_jump_vel)];
        break;
      default:
        my_assert(_dino_state == DINO_RUN);
        _curr_dino_frame = &dino_running_bitmap[_dino_ani_frame];
        break;
    }
  } else {
    _dino_jump_vel--;
    if (_dino_jump_vel == DINO_INITIAL_VEL) {
      _curr_dino_frame = &dino_running_bitmap[_dino_ani_frame];
      _dino_state = DINO_RUN;
    } else {
      _curr_dino_frame = &dino_jumping_bitmap[abs(_dino_jump_vel)];
    }
  }

  if (dinoDied()) {
    _game_over = true;
    _score_show_time = 0;
    gameOver();
    return;
  }
  printFrame();
  _dino_ani_frame ^= 1;
  if (_score < 9999) _score++;
}

inline void DinoApp::writeObsByte(uint8_t pos, display_buf_t line) {
  _obstacle_frame[pos] = line;
}

inline void DinoApp::printFrame() {
  display_buf_t frame[DISPLAY_WIDTH] = {0};
  memcpy(frame, _obstacle_frame, sizeof(frame));
  for (uint8_t i = 0; i < DINO_WIDTH; i++) {
    frame[i] |= (*_curr_dino_frame)[i];
  }
  display_set_mode_fixed_packed(frame);
}

bool DinoApp::dinoDied() {
  for (uint8_t i = 0; i < DINO_WIDTH; i++) {
    if (_obstacle_frame[i] & (*_curr_dino_frame)[i]) {
      return true;
    }
  }
  return false;
}

inline void DinoApp::gameOver() {
  show_score_app.SetScore(_score);
  g_game_score.MarkScore(GameScoreType::GAME_DINO, _score);
  badge_controller.change_app(&show_score_app);
}

}  // namespace dino
}  // namespace app
}  // namespace hitcon
