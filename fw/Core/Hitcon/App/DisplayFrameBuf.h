#ifndef DISPLAY_BUF_APP_H
#define DISPLAY_BUF_APP_H

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/RandomPool.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>

#include "app.h"

constexpr int UPDATE_PRIORITY = 960;
constexpr int UPDATE_INTERVAL = 50;       // ms
constexpr int DEFAULT_MOVE_PERIOD = 500;  // ms

namespace hitcon {

namespace app {

namespace display_frame_buf {

class DisplayFrameBuf : public App {
 private:
  const display_buf_t _display[45] = {
      0x61, /* 10000110 */
      0x51, /* 10001010 */
      0x49, /* 10010010 */
      0x45, /* 10100010 */
      0x43, /* 11000010 */
      0x0,  /* 00000000 */
      0x38, /* 00011100 */
      0x54, /* 00101010 */
      0x54, /* 00101010 */
      0x54, /* 00101010 */
      0x58, /* 00011010 */
      0x0,  /* 00000000 */
      0x61, /* 10000110 */
      0x51, /* 10001010 */
      0x49, /* 10010010 */
      0x45, /* 10100010 */
      0x43, /* 11000010 */
      0x0,  /* 00000000 */
      0x38, /* 00011100 */
      0x54, /* 00101010 */
      0x54, /* 00101010 */
      0x54, /* 00101010 */
      0x58, /* 00011010 */
      0x0,  /* 00000000 */
      0x44, /* 00100010 */
      0x3f, /* 11111100 */
      0x84, /* 00100001 */
      0x7c, /* 00111110 */
      0x0,  /* 00000000 */
      0x7c, /* 00111110 */
      0x44, /* 00100010 */
      0x7c, /* 00111110 */
      0x0,  /* 00000000 */
      0x89, /* 10010001 */
      0x52, /* 01001010 */
      0x0,  /* 00000000 */
      0xf8, /* 00011111 */
      0xa8, /* 00010101 */
      0xfe, /* 01111111 */
      0xa8, /* 00010101 */
      0xf8, /* 00011111 */
      0x0,  /* 00000000 */
      0x5e, /* 01111010 */
      0x0,  /* 00000000 */
      0x5e, /* 01111010 */
  };

 public:
  DisplayFrameBuf() = default;
  virtual ~DisplayFrameBuf() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern DisplayFrameBuf display_frame_buf_app;

}  // namespace display_frame_buf

}  // namespace app

}  // namespace hitcon

#endif  // DISPLAY_BUF_APP_H
