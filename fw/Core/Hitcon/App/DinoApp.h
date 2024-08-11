#ifndef DINO_APP_H
#define DINO_APP_H

#define DINO_WIDTH 5
#define DINO_HEIGHT 4
#define DINO_DOWN_HEIGHT 3
// The least distance of obsticals for contious jumping without game over
#define DINO_OBS_LEAST_DISTANCE 4
#define OBSTACAL_FRAME_WIDTH (DISPLAY_WIDTH + 3)
#define DINO_INITIAL_VEL (-DINO_HEIGHT)
#define DINO_SHOW_SCORE_TIME 60

#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "app.h"

using namespace hitcon::service::sched;

namespace hitcon {
namespace app {
namespace dino {

class DinoApp : public App {
 private:
  PeriodicTask _routine_task;
  static constexpr unsigned INTERVAL = 150;
  display_buf_t _obstacle_frame[OBSTACAL_FRAME_WIDTH] = {0};
  const unsigned char (*_curr_dino_frame)[5];
  int8_t _obstacle_interval;
  uint8_t _generate_obs_rate;
  uint16_t _score;
  uint8_t _score_show_time;
  uint8_t _game_over : 1;
  uint8_t _dino_ani_frame : 1;
  int8_t _dino_jump_vel : 3;
  enum dino_actions { DINO_RUN, DINO_JUMP, DINO_CRUNCH } _dino_state;
  void Routine(void* unused);
  bool dinoDied();
  void printFrame();
  void writeObsByte(uint8_t pos, display_buf_t line);
  void gameOver();

 public:
  DinoApp();
  virtual ~DinoApp() = default;
  void Init();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void OnEdgeButton(button_t button) override;
};

extern DinoApp dino_app;

}  // namespace dino
}  // namespace app
}  // namespace hitcon

constexpr display_buf_t dino_running_bitmap[2][5] = {
    {
        0x40, /* 01000000 */
        0x80, /* 10000000 */
        0xf0, /* 11110000 */
        0x30, /* 00110000 */
        0xb0, /* 10110000 */
    },
    {
        0x60, /* 01100000 */
        0x80, /* 10000000 */
        0x70, /* 01110000 */
        0xb0, /* 10110000 */
        0x30, /* 00110000 */
    },
};

constexpr display_buf_t dino_crunching_bitmap[2][5] = {
    {
        0xc0, /* 11000000 */
        0x80, /* 10000000 */
        0x60, /* 01100000 */
        0xe0, /* 11100000 */
        0x60, /* 01100000 */
    },
    {
        0x60, /* 01100000 */
        0x80, /* 10000000 */
        0xe0, /* 11100000 */
        0x60, /* 01100000 */
        0xe0, /* 11100000 */
    },
};

constexpr display_buf_t dino_jumping_bitmap[4][5] = {
    {
        0x01, /* 00000001 */
        0x02, /* 00000010 */
        0x03, /* 00000011 */
        0x00, /* 00000000 */
        0x02, /* 00000010 */
    },
    {
        0x01, /* 00000001 */
        0x02, /* 00000010 */
        0x03, /* 00000011 */
        0x00, /* 00000000 */
        0x02, /* 00000010 */
    },
    {
        0x02, /* 00000010 */
        0x04, /* 00000100 */
        0x07, /* 00000111 */
        0x01, /* 00000001 */
        0x05, /* 00000101 */
    },
    {
        0x08, /* 00001000 */
        0x10, /* 00010000 */
        0x1e, /* 00011110 */
        0x06, /* 00000110 */
        0x16, /* 00010110 */
    },
};

constexpr display_buf_t bird_bitmap[3] = {
    0x08, /* 00001000 */
    0x1c, /* 00011100 */
    0x14, /* 00010100 */
};

constexpr display_buf_t big_cactus_bitmap[3] = {
    0xc0, /* 11000000 */
    0xe0, /* 11100000 */
    0x60, /* 01100000 */
};

constexpr display_buf_t small_tall_cactus_bitmap[1] = {
    0xf0, /* 11110000 */
};

constexpr display_buf_t small_short_cactus_bitmap[1] = {
    0xe0, /* 11100000 */
};

#endif  // DINO_APP_H
