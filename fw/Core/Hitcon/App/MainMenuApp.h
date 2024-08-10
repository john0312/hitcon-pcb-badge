#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::snake::SetSingleplayer;

constexpr menu_entry_t main_menu_entries[] = {
    // TODO : change app
    {"BadUSB", nullptr, nullptr},
    {"Snake", &snake_app, &SetSingleplayer},
    {"Dino", nullptr, nullptr},
    {"Tetris", nullptr, nullptr},
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