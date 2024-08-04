#include "DinoApp.h"

#include <App/MainMenuApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>

#include <cmath>

using namespace hitcon::service::sched;

namespace hitcon {
DinoApp dino_app;

DinoApp::DinoApp()
    : _routine_task(930, (task_callback_t)&DinoApp::Routine, (void*)this,
                    INTERVAL) {}

void DinoApp::OnEntry() {
  memset(_obstacle_frame, 0, OBSTACAL_FRAME_WIDTH * sizeof(uint8_t));
  _input_buf = BUTTON_NONE;
  _curr_dino_frame = &dino_running_bitmap[0];
  _obstacle_interval = 0;
  _generate_obs_rate = 8;
  _obstacle_frame_pos = 0;
  _game_over = 0;
  _is_dino_down = 0;
  _dino_ani_frame = 0;
  _dino_jump_vel = DINO_INITIAL_VEL;
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
}

void DinoApp::OnExit() { scheduler.DisablePeriodic(&_routine_task); }

void DinoApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_DOWN:
      _input_buf = BUTTON_DOWN;
      break;
    case BUTTON_UP:
      _input_buf = BUTTON_UP;
      break;
  }
}

void DinoApp::Routine(void* unused) {
  if (_game_over) return;
  button_t _input = _input_buf;
  _input_buf = BUTTON_NONE;
  // Shift frame
  _obstacle_frame_pos = (_obstacle_frame_pos + 1) % (DISPLAY_WIDTH + 3);
  _obstacle_interval++;
  _is_dino_down = 0;
  // Generate obstacle
  if (_obstacle_interval > DINO_OBS_LEAST_DISTANCE) {
    if ((_obstacle_interval > DISPLAY_WIDTH * 2) ||
        (g_fast_random_pool.GetRandom() % _generate_obs_rate == 0)) {
      switch (g_fast_random_pool.GetRandom() % 4) {
        case 0:  // bird
          writeObsByte(_obstacle_frame_pos, bird_bitmap[0]);
          writeObsByte(_obstacle_frame_pos + 1, bird_bitmap[1]);
          writeObsByte(_obstacle_frame_pos + 2, bird_bitmap[2]);
          break;
        case 1:  // big cactuar
          writeObsByte(_obstacle_frame_pos, big_cactuar_bitmap[0]);
          writeObsByte(_obstacle_frame_pos + 1, big_cactuar_bitmap[1]);
          writeObsByte(_obstacle_frame_pos + 2, big_cactuar_bitmap[2]);
        case 2:  // small tall cactuar
          writeObsByte(_obstacle_frame_pos, small_tall_cactuar_bitmap[0]);
        case 3:  // small short cactuar
          writeObsByte(_obstacle_frame_pos, small_short_cactuar_bitmap[0]);
        default:
          my_assert(false);
      }
    }
  }
  // Determine dinosaur movement
  // Input is only available when dinosaur is on the ground
  if (_dino_jump_vel == DINO_INITIAL_VEL) {
    switch (_input) {
      case BUTTON_DOWN:
        _is_dino_down = 1;
        _curr_dino_frame = &dino_crunching_bitmap[_dino_ani_frame];
        break;
      case BUTTON_UP:
        _dino_jump_vel = DINO_HEIGHT - 1;
        _curr_dino_frame = &dino_jumping_bitmap[abs(_dino_jump_vel)];
      default:
        _curr_dino_frame = &dino_running_bitmap[_dino_ani_frame];
        break;
    }
  } else {
    _dino_jump_vel--;
    _curr_dino_frame = &dino_jumping_bitmap[abs(_dino_jump_vel)];
  }

  if (dinoDied()) {
    _game_over = true;
    badge_controller.change_app(&main_menu);
    return;
  }
  printFrame();
  _dino_ani_frame ^= 1;
}

inline void DinoApp::writeObsByte(uint8_t pos, display_buf_t line) {
  _obstacle_frame[pos] = line;
  if (pos >= (DISPLAY_WIDTH + 3))
    _obstacle_frame[pos - (DISPLAY_WIDTH + 3)] = line;
}

inline void DinoApp::printFrame() {
  display_buf_t frame[DISPLAY_WIDTH] = {0};
  memcpy(frame, _obstacle_frame + _obstacle_frame_pos, sizeof(frame));
  for (uint8_t i = 0; i < DINO_WIDTH; i++) {
    frame[i] |= (*_curr_dino_frame)[i];
  }
  display_set_mode_fixed_packed(frame);
}

bool DinoApp::dinoDied() {
  for (uint8_t i = 0; i < DINO_WIDTH; i++) {
    if (_obstacle_frame[_obstacle_frame_pos + i] & (*_curr_dino_frame)[i]) {
      return true;
    }
  }
  return false;
}

}  // namespace hitcon
