#include <App/SnakeApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::snake::SetMultiplayer;

constexpr menu_entry_t connect_menu_entries[] = {
    {"Tetris", &tetris_app, nullptr},
    {"Snake", &snake_app, &SetMultiplayer},
    {"xchg", nullptr, nullptr},
};

constexpr int connect_menu_entries_len =
    sizeof(connect_menu_entries) / sizeof(menu_entry_t);

class ConnectMenuApp : public MenuApp {
 public:
  ConnectMenuApp() : MenuApp(connect_menu_entries, connect_menu_entries_len) {}
  void OnButtonMode() override {}
};

extern ConnectMenuApp connect_menu;

}  // namespace hitcon
