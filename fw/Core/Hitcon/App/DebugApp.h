#include <App/BadUsbApp.h>
#include <App/BouncingDVDApp.h>
#include <App/DinoApp.h>
#include <App/ScoreHistApp.h>
#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/TamaApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

constexpr menu_entry_t debug_menu_entries[] = {{"Accel", nullptr, nullptr}};

constexpr size_t debug_menu_entries_len =
    sizeof(debug_menu_entries) / sizeof(debug_menu_entries[0]);

class DebugApp : public MenuApp {
 public:
  DebugApp() : MenuApp(debug_menu_entries, debug_menu_entries_len) {}

  void OnEntry() override {};

  void OnButtonMode() override {}
  void OnButtonBack() override { badge_controller.BackToMenu(this); }
  void OnButtonLongBack() override { badge_controller.BackToMenu(this); }
};

extern DebugApp g_debug_app;

}  // namespace hitcon
