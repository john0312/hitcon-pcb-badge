#include "BouncingDVDApp.h"

namespace hitcon {

namespace app {

namespace bouncing_dvd {

void BouncingDVD::update(int now) {
  if (now - last_update_time < move_period) {
    return;
  }

  last_update_time = now;

  int new_x = x + dx;
  int new_y = y + dy;
  bool cross_border = false;
  if (!(0 <= new_x && (new_x + FONT_WIDTH - 1) < DISPLAY_WIDTH)) {
    dx = -dx;
    cross_border = true;
  }
  if (!(0 <= new_y && (new_y + FONT_HEIGHT - 1) < DISPLAY_HEIGHT)) {
    dy = -dy;
    cross_border = true;
  }
  if (cross_border) {
    current_char = (current_char + 1) % TEXT_LENGTH;
  }

  if (!cross_border) {
    x = new_x;
    y = new_y;
  }

  // If the character is at the corner, drift it by 1 pixel randomly so
  // the audience won't be satisfied (:woozy_face:)
  if (x == 0 && y == 0) {
    if (__rand() % 2 == 0)
      x += __rand() % 5 == 0 ? 1 : 0;
    else
      y += __rand() % 5 == 0 ? 1 : 0;
  } else if (x == 0 && y == DISPLAY_HEIGHT - FONT_HEIGHT) {
    if (__rand() % 2 == 0)
      x += __rand() % 5 == 0 ? 1 : 0;
    else
      y -= __rand() % 5 == 0 ? 1 : 0;
  } else if (x == DISPLAY_WIDTH - FONT_WIDTH && y == 0) {
    if (__rand() % 2 == 0)
      x -= __rand() % 5 == 0 ? 1 : 0;
    else
      y += __rand() % 5 == 0 ? 1 : 0;
  } else if (x == DISPLAY_WIDTH - FONT_WIDTH &&
             y == DISPLAY_HEIGHT - FONT_HEIGHT) {
    if (__rand() % 2 == 0)
      x -= __rand() % 5 == 0 ? 1 : 0;
    else
      y -= __rand() % 5 == 0 ? 1 : 0;
  }
}

void BouncingDVD::draw(display_buf_t *buf) {
  memset(buf, 0, sizeof(display_buf_t) * DISPLAY_WIDTH);

  for (int i = 0; i < FONT_HEIGHT; i++) {
    for (int j = 0; j < FONT_WIDTH; j++) {
      display_buf_assign(buf[x + j], y + i, FONT[current_char][i][j]);
    }
  }
}

#ifndef HITCON_TEST_MODE

BouncingDVDApp bouncing_dvd_app;

using hitcon::service::sched::task_callback_t;

unsigned bouncing_dvd_rand() { return g_fast_random_pool.GetRandom(); }

BouncingDVDApp::BouncingDVDApp()
    : periodic_task(UPDATE_PRIORITY, (task_callback_t)&periodic_task_callback,
                    this, UPDATE_INTERVAL),
      bouncing_dvd(bouncing_dvd_rand) {
  hitcon::service::sched::scheduler.Queue(&periodic_task, nullptr);
}

void BouncingDVDApp::OnEntry() {
  hitcon::service::sched::scheduler.EnablePeriodic(&periodic_task);
}

void BouncingDVDApp::OnExit() {
  hitcon::service::sched::scheduler.DisablePeriodic(&periodic_task);
}

void BouncingDVDApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_UP:
      bouncing_dvd.inc_move_speed();
      break;

    case BUTTON_DOWN:
      bouncing_dvd.dec_move_speed();
      break;

    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      break;

    default:
      break;
  }
}

void BouncingDVDApp::periodic_task_callback(void *) {
  using hitcon::service::sched::SysTimer;

  int now = static_cast<int>(SysTimer::GetTime());
  bouncing_dvd.update(now);

  display_buf_t display_buf[DISPLAY_WIDTH];
  bouncing_dvd.draw(display_buf);
  display_set_mode_fixed_packed(display_buf);
}

#endif

}  // namespace bouncing_dvd

}  // namespace app

}  // namespace hitcon
