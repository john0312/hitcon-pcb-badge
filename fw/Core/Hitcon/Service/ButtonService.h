#ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
#define HITCON_SERVICE_BUTTON_SERVICE_H_

#include <Util/callback.h>

namespace hitcon {

/**
 * - If a button is pressed for <30ms or within 100ms of the last release, it is filtered as bounce.
 * - If a button is pressed for >30ms but <800ms, on release it'll be considered a press.
 * - If a button is pressed for >800ms but <3000ms, on release it'll be considered a long press.
 * - If a button is pressed for >3000ms, then repeat firing of press event will happen every 250ms until released.
 */

constexpr int BUTTON_VALUE_MASK = 0b1111;
constexpr int BUTTON_LONG_PRESS_BIT = 1 << 14;

// Bit 14 - This is a long press if set.
// Bit 0-3 - The button pressed. See BUTTON_* constants below.
enum button_t {
  BUTTON_UP = 1,
  BUTTON_DOWN = 2,
  BUTTON_LEFT = 3,
  BUTTON_RIGHT = 4,
  BUTTON_BACK = 5,
  BUTTON_OK = 6,
  BUTTON_MODE = 7,
  BUTTON_BRIGHTNESS = 8,

  BUTTON_LONG_UP = 1 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_DOWN = 2 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_LEFT = 3 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_RIGHT = 4 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_BACK = 5 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_OK = 6 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_MODE = 7 | BUTTON_LONG_PRESS_BIT,
  BUTTON_LONG_BRIGHTNESS = 8 | BUTTON_LONG_PRESS_BIT,
};


class ButtonService {
public:
  ButtonService();
  // Whenever a button press is pressed, the callback will be called,
  // with callback_arg1 and the button_t (cast to void*).
  void SetCallback(callback_t callback, void *callback_arg1);
};

extern ButtonService button_service;

} // namespace hitcon

#endif // #ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
