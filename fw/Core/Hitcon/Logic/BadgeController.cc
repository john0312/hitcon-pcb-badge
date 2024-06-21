#include "BadgeController.h"
#include <App/ShowNameApp.h>
#include <cstdint>

namespace hitcon {

BadgeController::BadgeController() {
  button_service.SetCallback(OnButton, this);
  current_app = &show_name_app;
  current_app->OnEntry();
}

void BadgeController::change_app(App *new_app) {
  current_app->OnExit();
  current_app = new_app;
  current_app->OnEntry();
}

void BadgeController::OnButton(void *arg1, void *arg2) {
  BadgeController *this_ = static_cast<BadgeController *>(arg1);
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg2));

  switch (button) {
  case BUTTON_MODE:
    // TODO: if button means change app, call change_app()
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
