#include "TetrisApp.h"

#include <App/MenuApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>

using hitcon::service::sched::SysTimer;
using hitcon::service::sched::task_callback_t;

namespace hitcon {

namespace {

unsigned int tetris_random() { return g_fast_random_pool.GetRandom(); }

}  // namespace

TetrisApp tetris_app;

TetrisApp::TetrisApp()
    : periodic_task(hitcon::tetris::UPDATE_PRIORITY,
                    (task_callback_t)&TetrisApp::periodic_task_callback, this,
                    hitcon::tetris::UPDATE_INTERVAL),
      game(tetris_random) {
  hitcon::service::sched::scheduler.Queue(&periodic_task, nullptr);
}

void TetrisApp::OnEntry() {
  // start the update task
  hitcon::service::sched::scheduler.EnablePeriodic(&periodic_task);
}

void TetrisApp::OnExit() {
  hitcon::service::sched::scheduler.DisablePeriodic(&periodic_task);
}

void TetrisApp::OnButton(button_t button) {
  if (game.game_is_over()) {
    // exit the app
    badge_controller.OnAppEnd(this);

  } else {
    /**
     * Note that we need to rotate the badge by 90 degrees clockwise to play the
     * game. Therefore, the button is remapped.
     */
    switch (button) {
      case BUTTON_LEFT:
        game.game_on_input(hitcon::tetris::DIRECTION_UP);
        break;

      case BUTTON_RIGHT:
        game.game_on_input(hitcon::tetris::DIRECTION_DOWN);
        break;

      case BUTTON_DOWN:
        game.game_on_input(hitcon::tetris::DIRECTION_LEFT);
        break;

      case BUTTON_UP:
        game.game_on_input(hitcon::tetris::DIRECTION_RIGHT);
        break;

      default:
        break;
    }
  }
}

void TetrisApp::periodic_task_callback(void *) {
  if (game.game_is_over()) {
    // TODO: maybe score?
  } else {
    // check if it's time to fall down
    unsigned now = SysTimer::GetTime();
    if (now - last_fall_time >= hitcon::tetris::FALL_PERIOD) {
      game.game_fall_down_tetromino();
      last_fall_time = now;
    }

    // update display buffer
    display_buf_t display_buf[DISPLAY_WIDTH];
    game.game_draw_to_display(display_buf);
    display_set_mode_fixed_packed(display_buf);
  }
}

}  // namespace hitcon
