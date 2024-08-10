#include "BadgeController.h"

#include <App/EditNameApp.h>
#include <App/ShowNameApp.h>
#include <Service/Sched/Checks.h>

#include <cstdint>

using hitcon::service::sched::my_assert;

namespace hitcon {
BadgeController badge_controller;

BadgeController::BadgeController() : current_app(nullptr) {}

void BadgeController::Init() {
  g_button_logic.SetCallback((callback_t)&BadgeController::OnButton, this);
  g_button_logic.SetEdgeCallback((callback_t)&BadgeController::OnEdgeButton, this);
  current_app = &show_name_app;
  current_app->OnEntry();
}

void BadgeController::change_app(App *new_app) {
  if (current_app) current_app->OnExit();
  current_app = new_app;
  if (current_app) current_app->OnEntry();
}

void BadgeController::OnAppEnd(App *ending_app) {
  my_assert(current_app == ending_app);

  // TODO: Change this.
  change_app(&show_name_app);
}

void BadgeController::OnButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));

  if (button == BUTTON_BRIGHTNESS || button == BUTTON_LONG_BRIGHTNESS) {
    // TODO: change brightness
  }

  // forward the button to the current app
  current_app->OnButton(button);
}

void BadgeController::OnXBoardConnect(void *unused) {
  // TODO
}

void BadgeController::OnXBoardDisconnect(void *unused) {
  // TODO
}

void BadgeController::OnEdgeButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));
  current_app->OnEdgeButton(button);
}

}  // namespace hitcon
