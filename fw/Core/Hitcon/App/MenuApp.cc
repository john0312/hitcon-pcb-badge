#include "MenuApp.h"

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

namespace hitcon {

struct menu_entry_t {
  const char *name;
  App *app;
};
menu_entry_t menu_entries[] = {
    {"game", /* TODO */ &show_name_app},
    {"USB", /* TODO */ &show_name_app},
    {"infrared", /* TODO */ &show_name_app},
};
int menu_entry_index;

MenuApp menu_app;

MenuApp::MenuApp() { menu_entry_index = 0; }

void MenuApp::OnEntry() {}

void MenuApp::OnExit() {}

void MenuApp::OnButton(button_t button) {
  switch (button) {
    // go back to show_name_app
    case BUTTON_MODE:
      badge_controller.change_app(&show_name_app);
      break;

    // button up and down to focus on other apps
    case BUTTON_UP:
      menu_entry_index++;
      if (menu_entry_index >= sizeof(menu_entries) / sizeof(menu_entry_t)) {
        menu_entry_index = 0;
      }
      // TODO: display the text of the focused app
      // should create a new api: display_set_menu_text(const char *text)
      break;
    case BUTTON_DOWN:
      menu_entry_index--;
      if (menu_entry_index < 0) {
        menu_entry_index = sizeof(menu_entries) / sizeof(menu_entry_t) - 1;
      }
      // TODO: display the text of the focused app
      break;

    case BUTTON_OK:
      badge_controller.change_app(menu_entries[menu_entry_index].app);
      break;
  }
}

}  // namespace hitcon
