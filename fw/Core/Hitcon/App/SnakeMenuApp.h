#ifndef SNAKE_MENU_APP_H
#define SNAKE_MENU_APP_H

#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {
namespace app {
namespace snake {

constexpr menu_entry_t snake_menu_entries[] = {
    {"Single Player", &snake_app, &SetSingleplayer},
    {"Multi Player", &snake_app, &SetMultiplayer},
};

constexpr int snake_menu_entries_len =
    sizeof(snake_menu_entries) / sizeof(menu_entry_t);

class SnakeMenuApp : public MenuApp {
 public:
  SnakeMenuApp() : MenuApp(snake_menu_entries, snake_menu_entries_len) {}

  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
};

extern SnakeMenuApp snake_menu;
}  // namespace snake
}  // namespace app
}  // namespace hitcon

#endif
