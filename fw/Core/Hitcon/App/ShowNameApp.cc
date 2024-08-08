#include "ShowNameApp.h"

#include <App/MainMenuApp.h>
#include <App/NameSettingApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/Display/font.h>
#include <Logic/GameLogic.h>
#include <Service/Sched/Task.h>

#include <cstdio>
#include <cstring>

using namespace hitcon::service::sched;
using hitcon::game::gameLogic;

namespace hitcon {
ShowNameApp show_name_app;

ShowNameApp::ShowNameApp()
    : _routine_task(490, (task_callback_t)&ShowNameApp::check_update, this,
                    1000) {
  strncpy(name, DEFAULT_NAME, NAME_LEN);
}

void ShowNameApp::OnEntry() {
  display_set_mode_scroll_text(name);
  scheduler.Queue(&_routine_task, nullptr);
}

void ShowNameApp::OnExit() {}

void ShowNameApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_LONG_MODE:
      badge_controller.change_app(&name_setting_menu);
      break;

    case BUTTON_MODE:
      badge_controller.change_app(&main_menu);
      break;
  }
}

void ShowNameApp::check_update() {
  if (score_cache != gameLogic.get_cache().total_score) {
    score_cache = gameLogic.get_cache().total_score;
    update_display();
  }
}

void ShowNameApp::update_display() {
  constexpr int max_len = DISPLAY_SCROLL_MAX_COLUMNS / CHAR_WIDTH;
  char display_str[max_len];

  switch (mode) {
    case NameScore:
      snprintf(display_str, max_len, "%s%ld\n", name, score_cache);
      break;
    case NameOnly:
      snprintf(display_str, max_len, "%s\n", name);
      break;
    case ScoreOnly:
      snprintf(display_str, max_len, "%d\n", score_cache);
      break;
    default:
      break;
  }
  display_set_mode_scroll_text(display_str);
}

void ShowNameApp::SetName(const char *name) {
  strncpy(this->name, name, NAME_LEN);
  update_display();
}

void ShowNameApp::SetMode(const enum ShowNameMode mode) {
  this->mode = mode;
  update_display();
}

enum ShowNameMode ShowNameApp::GetMode() { return mode; }

}  // namespace hitcon
