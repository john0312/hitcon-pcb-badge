#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::snake::snake_app;
using hitcon::app::tetris::tetris_app;
// using hitcon::app::tetris

constexpr menu_entry_t connect_menu_entries[] = {
    {"Tetris", &tetris_app, &hitcon::app::tetris::SetMultiplayer},
    {"Snake", &snake_app, &hitcon::app::snake::SetMultiplayer},
};

constexpr int connect_menu_entries_len =
    sizeof(connect_menu_entries) / sizeof(menu_entry_t);

class ConnectMenuApp : public MenuApp {
 public:
  ConnectMenuApp() : MenuApp(connect_menu_entries, connect_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern ConnectMenuApp connect_menu;

}  // namespace hitcon
