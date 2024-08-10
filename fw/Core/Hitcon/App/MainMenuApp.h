#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

constexpr menu_entry_t main_menu_entries[] = {
    // TODO : change app
    {"BadUSB", &show_name_app, nullptr},
    {"Snake", &show_name_app, nullptr},
    {"Dino", &show_name_app, nullptr},
    {"Tetris", &show_name_app, nullptr},
};

constexpr int main_menu_entries_len =
    sizeof(main_menu_entries) / sizeof(menu_entry_t);

class MainMenuApp : public MenuApp {
 public:
  MainMenuApp() : MenuApp(main_menu_entries, main_menu_entries_len) {}
  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
};

extern MainMenuApp main_menu;

}  // namespace hitcon