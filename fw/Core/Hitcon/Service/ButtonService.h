#ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
#define HITCON_SERVICE_BUTTON_SERVICE_H_

#include <Util/callback.h>

namespace hitcon {

// Bit 14 - This is a long press if set.
// Bit 0-3 - The button pressed. See BUTTON_* constants below.
typedef int button_t;

class ButtonService {
 public:
  ButtonService();

  // If a button is pressed for <30ms or within 100ms of the last release, it is filtered as bounce.
  // If a button is pressed for >30ms but <800ms, on release it'll be considered a press.
  // If a button is pressed for >800ms but <3000ms, on release it'll be considered a long press.
  // If a button is pressed for >3000ms, then repeat firing of press event will happen every 250ms until released.

  // Whenever a button press is pressed, the callback will be called,
  // with callback_arg1 and the button_t (cast to void*).
  void SetCallback(callback_t callback, void* callback_arg1);

  static constexpr button_t BUTTON_UP = 1;
  static constexpr button_t BUTTON_DOWN = 2;
  static constexpr button_t BUTTON_LEFT = 3;
  static constexpr button_t BUTTON_RIGHT = 4;
  static constexpr button_t BUTTON_BACK = 5;
  static constexpr button_t BUTTON_OK = 6;
  static constexpr button_t BUTTON_MODE = 7;
  static constexpr button_t BUTTON_BRIGHTNESS = 8;
};

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
