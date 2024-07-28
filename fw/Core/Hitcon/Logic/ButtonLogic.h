#ifndef HITCON_LOGIC_BUTTON_LOGIC_H_
#define HITCON_LOGIC_BUTTON_LOGIC_H_

#include <Service/ButtonService.h>
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <main.h>
#include <stdint.h>

namespace hitcon {

/**
 * - If a button is pressed for <30ms or within 100ms of the last release, it is
 * filtered as bounce.
 * - If a button is pressed for >30ms but <800ms, on release it'll be considered
 * a press.
 * - If a button is pressed for >800ms but <1500ms, on release it'll be
 * considered a long press.
 * - If a button is pressed for >1500ms, then repeat firing of press event will
 * happen every 200ms until released.
 */
// Long Press Threshold 800ms
constexpr int LONG_PRESS_TIME_THRESHOLD = 80;
// Bounce Time Threshold 30ms
constexpr int BOUNCE_TIME_THRESHOLD = 3;

constexpr int BUTTON_VALUE_MASK = 0b1111;
constexpr int BUTTON_LONG_PRESS_BIT = 1 << 14;

// Bit 14 - This is a long press if set.
// Bit 0-3 - The button pressed. See BUTTON_* constants below.
enum button_t {
  BUTTON_MODE = 1,
  BUTTON_DOWN = 2,
  BUTTON_BRIGHTNESS = 3,
  BUTTON_LEFT = 4,
  BUTTON_OK = 5,
  BUTTON_RIGHT = 6,
  BUTTON_BACK = 7,
  BUTTON_UP = 8,

  BUTTON_LONG_MODE = 1 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_DOWN = 2 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_BRIGHTNESS = 3 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_LEFT = 4 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_OK = 5 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_RIGHT = 6 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_BACK = 7 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_UP = 8 | BUTTON_LONG_PRESS_BIT,
};

class ButtonLogic {
 public:
  ButtonLogic();

  void Init();

  // Whenever a button press is pressed, the callback will be called,
  // with callback_arg1 and the button_t (cast to void*).
  void SetCallback(callback_t callback, void* callback_arg1);

  void OnReceiveData(uint8_t* arr);

  void CallbackWrapper(void* arg2);

 private:
  callback_t callback;
  void* callback_arg1;

  hitcon::service::sched::Task _callback_task;

  uint16_t _count[BUTTON_AMOUNT];
  uint8_t _fire;
  uint16_t _out;
};

extern ButtonLogic g_button_logic;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_BUTTON_LOGIC_H_
