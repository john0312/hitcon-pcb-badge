#include "DisplayFrameBuf.h"

#include <Logic/Display/display.h>

namespace hitcon {
namespace app {
namespace display_frame_buf {

DisplayFrameBuf display_frame_buf_app;

void DisplayFrameBuf::OnEntry() {
  display_set_orientation(0);
  display_set_mode_scroll_packed(_display,
                                 sizeof(_display) / sizeof(display_buf_t));
}

void DisplayFrameBuf::OnExit() { display_set_orientation(1); }

void DisplayFrameBuf::OnButton(button_t button) {
  switch (button) {
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      break;

    default:
      break;
  }
}

}  // namespace display_frame_buf
}  // namespace app
}  // namespace hitcon
