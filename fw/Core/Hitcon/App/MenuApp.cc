#include "MenuApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>

namespace hitcon {

MenuApp::MenuApp(const menu_entry_t *menu_entries, const int menu_entry_size)
    : menu_entries(menu_entries), menu_entry_size(menu_entry_size) {
  menu_entry_index = 0;
  active = false;
}

void MenuApp::OnEntry() {
  active = true;
  display_set_mode_scroll_text(menu_entries[menu_entry_index].name);
}

void MenuApp::OnExit() { active = false; }

void MenuApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_UP:
      menu_entry_index++;
      if (menu_entry_index >= menu_entry_size) {
        menu_entry_index = 0;
      }
      display_set_mode_scroll_text(menu_entries[menu_entry_index].name);
      break;

    case BUTTON_DOWN:
      menu_entry_index--;
      if (menu_entry_index < 0) {
        menu_entry_index = menu_entry_size - 1;
      }
      display_set_mode_scroll_text(menu_entries[menu_entry_index].name);
      break;

    case BUTTON_OK:
      if (menu_entries[menu_entry_index].func != nullptr) {
        (menu_entries[menu_entry_index].func)();
      }
      if (menu_entries[menu_entry_index].app != nullptr) {
        badge_controller.change_app(menu_entries[menu_entry_index].app);
      }
      break;

    case BUTTON_MODE:
      OnButtonMode();
      break;

    case BUTTON_BACK:
      OnButtonBack();
      break;

    case BUTTON_LONG_BACK:
      OnButtonLongBack();
      break;

    default:
      break;
  }
}

void MenuApp::AdjustMenuSize(int new_size, bool reset_index) {
  menu_entry_size = new_size;
  if (reset_index || menu_entry_index >= new_size) {
    menu_entry_index = 0;
    if (active)
      display_set_mode_scroll_text(menu_entries[menu_entry_index].name);
  }
}

void MenuApp::AdjustMenuPointer(const menu_entry_t *new_menu, int new_size,
                                bool reset_index) {
  menu_entries = new_menu;
  menu_entry_size = new_size;
  if (reset_index || menu_entry_index >= new_size) {
    menu_entry_index = 0;
  }
  if (active) display_set_mode_scroll_text(menu_entries[menu_entry_index].name);
}

}  // namespace hitcon
