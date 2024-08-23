#ifndef BOUNCING_DVD_APP_H
#define BOUNCING_DVD_APP_H

#include <Logic/Display/display.h>

#ifndef HITCON_TEST_MODE
#include <Logic/BadgeController.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>

#include "app.h"
#endif

namespace hitcon {

namespace app {

namespace bouncing_dvd {

#define HITCON_BOUNCING_SMALL

#ifdef HITCON_BOUNCING_SMALL
/**
 *   X.X   XXX   XXX   XXX   XXX   XXX
 * H XXX I .X. T .X. C X.. O X.X N X.X
 *   X.X   XXX   .X.   XXX   XXX   X.X
 */

constexpr int FONT_WIDTH = 3;
constexpr int FONT_HEIGHT = 3;
constexpr int FONT[][FONT_HEIGHT][FONT_WIDTH] = {
    {{1, 0, 1}, {1, 1, 1}, {1, 0, 1}},  // H
    {{1, 1, 1}, {0, 1, 0}, {1, 1, 1}},  // I
    {{1, 1, 1}, {0, 1, 0}, {0, 1, 0}},  // T
    {{1, 1, 1}, {1, 0, 0}, {1, 1, 1}},  // C
    {{1, 1, 1}, {1, 0, 1}, {1, 1, 1}},  // O
    {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}},  // N
};

#else  // HITCON_BOUNCING_BIG

/**
 *   X...X   XXXXX   XXXXX   XXXXX   XXXXX   X...X
 *   X...X   ..X..   ..X..   X....   X...X   XX..X
 * H XXXXX I ..X.. T ..X.. C X.... O X...X N X.X.X
 *   X...X   ..X..   ..X..   X....   X...X   X..XX
 *   X...X   XXXXX   ..X..   XXXXX   XXXXX   X...X
 */
constexpr int FONT_WIDTH = 5;
constexpr int FONT_HEIGHT = 5;
constexpr int FONT[][FONT_HEIGHT][FONT_WIDTH] = {
    {{1, 0, 0, 0, 1},
     {1, 0, 0, 0, 1},
     {1, 1, 1, 1, 1},
     {1, 0, 0, 0, 1},
     {1, 0, 0, 0, 1}},  // H
    {{1, 1, 1, 1, 1},
     {0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0},
     {1, 1, 1, 1, 1}},  // I
    {{1, 1, 1, 1, 1},
     {0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0}},  // T
    {{1, 1, 1, 1, 1},
     {1, 0, 0, 0, 0},
     {1, 0, 0, 0, 0},
     {1, 0, 0, 0, 0},
     {1, 1, 1, 1, 1}},  // C
    {{1, 1, 1, 1, 1},
     {1, 0, 0, 0, 1},
     {1, 0, 0, 0, 1},
     {1, 0, 0, 0, 1},
     {1, 1, 1, 1, 1}},  // O
    {{1, 0, 0, 0, 1},
     {1, 1, 0, 0, 1},
     {1, 0, 1, 0, 1},
     {1, 0, 0, 1, 1},
     {1, 0, 0, 0, 1}},  // N
};

#endif  // HITCON_BOUNCING_SMALL

constexpr int TEXT_LENGTH =
    sizeof(FONT) / (FONT_WIDTH * FONT_HEIGHT) / sizeof(int);
constexpr int UPDATE_PRIORITY = 960;
constexpr int UPDATE_INTERVAL = 50;       // ms
constexpr int DEFAULT_MOVE_PERIOD = 500;  // ms
inline void inc_move_speed(int &move_speed) {
  move_speed = move_speed * 9 / 10;
  if (move_speed < UPDATE_INTERVAL) {
    move_speed = UPDATE_INTERVAL;
  }
}
inline void dec_move_speed(int &move_speed) {
  move_speed = move_speed * 10 / 9;
  if (move_speed > 2000) {
    move_speed = 2000;
  }
}

class BouncingDVD {
 private:
  int x = DISPLAY_WIDTH / 2 - FONT_WIDTH / 2;
  int y = DISPLAY_HEIGHT / 2 - FONT_HEIGHT / 2;
  int dx = 1;
  int dy = 1;
  int move_period = DEFAULT_MOVE_PERIOD;
  int last_update_time = 0;
  int current_char = 0;

  unsigned (*__rand)(void) = nullptr;

 public:
  BouncingDVD() = default;
  BouncingDVD(unsigned (*rand)(void)) : __rand(rand) {}

  void update(int now);
  void draw(display_buf_t *buf);
  inline void inc_move_speed() {
    ::hitcon::app::bouncing_dvd::inc_move_speed(move_period);
  }
  inline void dec_move_speed() {
    ::hitcon::app::bouncing_dvd::dec_move_speed(move_period);
  }
};

#ifndef HITCON_TEST_MODE

/**
 * The app that displays a bouncing "HITCON" text.
 */
class BouncingDVDApp : public App {
 private:
  BouncingDVD bouncing_dvd;
  hitcon::service::sched::PeriodicTask periodic_task;

 public:
  BouncingDVDApp();
  virtual ~BouncingDVDApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void periodic_task_callback(void *);
};

extern BouncingDVDApp bouncing_dvd_app;

#endif  // HITCON_TEST_MODE

}  // namespace bouncing_dvd

}  // namespace app

}  // namespace hitcon

#endif  // BOUNCING_DVD_APP_H
