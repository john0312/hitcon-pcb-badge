#include "ShowNameApp.h"

#include <App/MainMenuApp.h>
#include <App/NameSettingApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/Display/font.h>
#include <Logic/GameLogic.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>
#include <Util/uint_to_str.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::game::gameLogic;

namespace hitcon {

namespace {

// Update once every 15s. Units: ms.
constexpr unsigned kMinUpdateInterval = 15 * 1000;

}  // namespace
ShowNameApp show_name_app;

ShowNameApp::ShowNameApp()
    : _routine_task(490, (task_callback_t)&ShowNameApp::check_update, this,
                    1000),
      last_disp_update(0) {}

void ShowNameApp::Init() {
  nv_storage_content &content = g_nv_storage.GetCurrentStorage();
  if (g_nv_storage.IsStorageValid() && strlen(content.name)) {
    strncpy(name, content.name, NAME_LEN);
  } else {
    strncpy(name, DEFAULT_NAME, NAME_LEN);
  }
  scheduler.Queue(&_routine_task, nullptr);
}

void ShowNameApp::OnEntry() {
  display_set_orientation(0);
  score_cache = gameLogic.GetScore();
  update_display();
  scheduler.EnablePeriodic(&_routine_task);
}

void ShowNameApp::OnExit() {
  display_set_orientation(1);
  scheduler.DisablePeriodic(&_routine_task);
}

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
  if (SysTimer::GetTime() - last_disp_update > kMinUpdateInterval) {
    if (score_cache != gameLogic.GetScore() && mode != NameOnly) {
      score_cache = gameLogic.GetScore();
      update_display();
    }
  }
}

void ShowNameApp::update_display() {
  constexpr int max_len = DISPLAY_SCROLL_MAX_COLUMNS / CHAR_WIDTH;

  last_disp_update = SysTimer::GetTime();

  static char display_str[max_len + 1];
  int name_len = strlen(name);

  static char num_str[max_len + 1];
  int num_len = 0;
  uint32_t score_ = score_cache;

  uint_to_chr(num_str, max_len + 1, score_);
  num_len = strlen(num_str);

  switch (mode) {
    case NameScore:
      if (name_len > max_len - num_len - 1) name_len = max_len - num_len - 1;
      strncpy(display_str, name, name_len);
      display_str[name_len] = '-';
      strncpy(display_str + name_len + 1, num_str, num_len);
      display_str[name_len + num_len + 1] = 0;
      break;
    case NameOnly:
      strncpy(display_str, name, name_len);
      display_str[name_len] = 0;
      break;
    case ScoreOnly:
      strncpy(display_str, num_str, num_len);
      display_str[num_len] = 0;
      break;
    default:
      break;
  }
  display_set_mode_scroll_text(display_str);
}

void ShowNameApp::SetName(const char *name) {
  strncpy(this->name, name, NAME_LEN);
  nv_storage_content &content = g_nv_storage.GetCurrentStorage();
  strncpy(content.name, name, NAME_LEN);
  g_nv_storage.MarkDirty();
  g_nv_storage.ForceFlush(nullptr, nullptr);
  update_display();
}

void ShowNameApp::SetMode(const enum ShowNameMode mode) {
  this->mode = mode;
  update_display();
}

enum ShowNameMode ShowNameApp::GetMode() { return mode; }

}  // namespace hitcon
