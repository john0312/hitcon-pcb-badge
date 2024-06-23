#include "BadgeController.h"
#include <App/ShowNameApp.h>
#include <cstdint>

namespace hitcon {

BadgeController::BadgeController() :
  current_app(nullptr) {
}

void BadgeController::Init() {
  button_service.SetCallback((callback_t)&BadgeController::OnButton, this);
  current_app = &show_name_app;
  current_app->OnEntry();
}

void BadgeController::change_app(App *new_app) {
  if (current_app) current_app->OnExit();
  current_app = new_app;
  if (current_app) current_app->OnEntry();
}

void BadgeController::OnButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));

  switch (button) {
  case BUTTON_BRIGHTNESS:
  case BUTTON_LONG_BRIGHTNESS:
    // TODO: change brightness
    break;

  default:
    // forward the button to the current app
    current_app->OnButton(button);
    break;
  }
}

} // namespace hitcon
