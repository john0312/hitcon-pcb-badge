#include "BadgeController.h"
#include <App/ShowEditNameApp.h>
#include <cstdint>

namespace hitcon {

BadgeController::BadgeController() {
  button_service.SetCallback(OnButton, this);
  current_app = &show_edit_name_app;
  current_app->OnEntry();
}

void BadgeController::OnButton(void *arg1, void *arg2) {
  BadgeController *this_ = static_cast<BadgeController *>(arg1);
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg2));

  switch (button) {
    case BUTTON_MODE:
      // TODO: if button means change app, change app (current_app.OnExit(),
      // new_app.OnEntry())
      break;

    case BUTTON_BRIGHTNESS:
    case BUTTON_LONG_BRIGHTNESS:
      // TODO: change brightness
      break;

    default:
      // forward the button to the current app
      this_->current_app->OnButton(button);
      break;
  }
}

} // namespace hitcon
