#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::snake::snake_app;
using hitcon::app::tetris::tetris_app;
// using hitcon::app::tetris

constexpr menu_entry_t menu_entries_24[] = {
    {"2024 badge", nullptr, nullptr},
};

constexpr int menu_entries_len_24 =
    sizeof(menu_entries_24) / sizeof(menu_entry_t);

class MenuApp24 : public MenuApp {
 public:
  MenuApp24() : MenuApp(menu_entries_24, menu_entries_len_24) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

constexpr menu_entry_t menu_entries_25_peer[] = {
    {"Tetris", &tetris_app, &hitcon::app::tetris::SetMultiplayer},
    {"Snake", &snake_app, &hitcon::app::snake::SetMultiplayer},
};

constexpr int menu_entries_len_25_peer =
    sizeof(menu_entries_25_peer) / sizeof(menu_entry_t);

class MenuApp25Peer : public MenuApp {
 public:
  MenuApp25Peer() : MenuApp(menu_entries_25_peer, menu_entries_len_25_peer) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

constexpr menu_entry_t menu_entries_25_base[] = {
    {"2025 base station", nullptr, nullptr},
};

constexpr int menu_entries_len_25_base =
    sizeof(menu_entries_25_base) / sizeof(menu_entry_t);

class MenuApp25Base : public MenuApp {
 public:
  MenuApp25Base() : MenuApp(menu_entries_25_base, menu_entries_len_25_base) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern MenuApp24 menu_24;
extern MenuApp25Peer menu_25_peer;
extern MenuApp25Base menu_25_base;

}  // namespace hitcon
