#include <App/BadUsbApp.h>
#include <App/BouncingDVDApp.h>
#include <App/DebugApp.h>
#include <App/DinoApp.h>
#include <App/ScoreHistApp.h>
#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/TamaApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::bouncing_dvd::bouncing_dvd_app;
using hitcon::app::dino::dino_app;
using hitcon::app::snake::snake_app;
using hitcon::app::tama::tama_app;
using hitcon::app::tetris::tetris_app;

constexpr menu_entry_t main_menu_entries[] = {
    // TODO : change app
    {"BadUSB", &hitcon::usb::bad_usb_app, nullptr},
    {"Snake", &snake_app, &hitcon::app::snake::SetSingleplayer},
    {"Dino", &dino_app, nullptr},
    {"Tetris", &tetris_app, &hitcon::app::tetris::SetSingleplayer},
    {"Show Scores", &score_hist::g_score_hist, nullptr},
    {"Bouncing DVD", &bouncing_dvd_app, nullptr},
    {"Tama", &tama_app, &hitcon::app::tama::SetSingleplayer},
    {"Debug", &g_debug_app, nullptr}};

constexpr int main_menu_entries_len =
    sizeof(main_menu_entries) / sizeof(menu_entry_t);

class MainMenuApp : public MenuApp {
 public:
  MainMenuApp() : MenuApp(main_menu_entries, main_menu_entries_len) {}
  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
  void OnButtonBack() override { badge_controller.change_app(&show_name_app); }
  void OnButtonLongBack() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern MainMenuApp main_menu;

}  // namespace hitcon
