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

constexpr menu_entry_t connect_legacy_menu_entries[] = {
    {"Unsupported", nullptr, nullptr},
};

constexpr int connect_legacy_menu_entries_len =
    sizeof(connect_legacy_menu_entries) / sizeof(menu_entry_t);

class ConnectLegacyMenuApp : public MenuApp {
 public:
  ConnectLegacyMenuApp()
      : MenuApp(connect_legacy_menu_entries, connect_legacy_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern ConnectLegacyMenuApp connect_legacy_menu;

constexpr menu_entry_t connect_basestn_menu_entries[] = {
    {"BaseStation", nullptr, nullptr},
};

constexpr int connect_basestn_menu_entries_len =
    sizeof(connect_basestn_menu_entries) / sizeof(menu_entry_t);

class ConnectBasestnMenuApp : public MenuApp {
 public:
  ConnectBasestnMenuApp()
      : MenuApp(connect_basestn_menu_entries,
                connect_basestn_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern ConnectBasestnMenuApp connect_basestn_menu;

}  // namespace hitcon
