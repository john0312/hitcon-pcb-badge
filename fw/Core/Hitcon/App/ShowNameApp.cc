#include "ShowNameApp.h"

#include <App/MainMenuApp.h>
#include <App/NameSettingApp.h>
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
      badge_controller.change_app(&name_setting_menu);
      break;

    case BUTTON_MODE:
      badge_controller.change_app(&main_menu);
      break;
  }
}

void ShowNameApp::SetName(const char *name) { strcpy(this->name, name); }

void ShowNameApp::SetMode(const enum ShowNameMode mode) { this->mode = mode; }

enum ShowNameMode ShowNameApp::GetMode() { return mode; }

}  // namespace hitcon
