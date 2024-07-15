#include "ShowNameApp.h"

#include <App/EditNameApp.h>
#include <App/MenuApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>

#include <cstring>

namespace hitcon {
ShowNameApp show_name_app;

ShowNameApp::ShowNameApp() { strcpy(name, DEFAULT_NAME); }

void ShowNameApp::OnEntry() { display_set_mode_scroll_text(name); }

void ShowNameApp::OnExit() {}

void ShowNameApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_LONG_MODE:
      badge_controller.change_app(&edit_name_app);
      break;

    case BUTTON_MODE:
      badge_controller.change_app(&menu_app);
      break;
  }
}

void ShowNameApp::SetName(const char *name) { strcpy(this->name, name); }

}  // namespace hitcon
