#include "ShowNameApp.h"

#include <App/ConnectMenuApp.h>
#include <App/MainMenuApp.h>
#include <App/NameSettingApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/Display/font.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>
#include <Util/uint_to_str.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::UsartConnectState;

namespace hitcon {

namespace {

// Update once every 15s. Units: ms.
constexpr unsigned kMinUpdateInterval = 15 * 1000;
static const char SURPRISE_NAME[] = "You got pwned!";
static const int SURPRISE_NAME_LEN = sizeof(SURPRISE_NAME) / sizeof(char);
static constexpr unsigned SURPRISE_TIME = 10 * 1000;

}  // namespace
ShowNameApp show_name_app;

ShowNameApp::ShowNameApp()
    : _routine_task(490, (task_callback_t)&ShowNameApp::check_update, this,
                    1000),
      last_disp_update(0), mode(SHOW_INITIALIZE) {}

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
  // TODO: update score with our new game
  // score_cache = gameLogic.GetScore();
  scheduler.EnablePeriodic(&_routine_task);
  starting_up = false;
  update_display();
}

void ShowNameApp::OnExit() {
  display_set_orientation(1);
  scheduler.DisablePeriodic(&_routine_task);
}

void ShowNameApp::OnButton(button_t button) {
  const UsartConnectState conn_state = g_xboard_logic.GetConnectState();
  switch (button) {
    case BUTTON_LONG_MODE:
      if (conn_state == UsartConnectState::ConnectPeer2025) {
        badge_controller.change_app(&connect_menu);
      } else if (conn_state == UsartConnectState::ConnectLegacy) {
        badge_controller.change_app(&connect_legacy_menu);
      } else if (conn_state == UsartConnectState::ConnectBaseStn2025) {
        badge_controller.change_app(&connect_basestn_menu);
      } else {
        badge_controller.change_app(&name_setting_menu);
      }
      break;

    case BUTTON_MODE:
      if (g_xboard_logic.GetConnectState() ==
          UsartConnectState::ConnectPeer2025) {
        badge_controller.change_app(&connect_menu);
      } else {
        badge_controller.change_app(&main_menu);
      }
      break;
  }
}

void ShowNameApp::check_update() {
  if (mode == SHOW_INITIALIZE) {
    // NOTE:if FR complete, load from NV storage
    mode = NameScore;
    update_display();
  } else if (mode == Surprise &&
             SysTimer::GetTime() - last_disp_update > SURPRISE_TIME) {
    mode = NameScore;
    update_display();
    badge_controller.RestoreApp();
  } else if (mode != Surprise &&
             (SysTimer::GetTime() - last_disp_update > kMinUpdateInterval ||
              starting_up)) {
    // TODO: check and update score with our new game
  }
}

void ShowNameApp::update_display() {
  constexpr int max_len = kDisplayScrollMaxTextLen;
  static char display_str[max_len + 1];

  last_disp_update = SysTimer::GetTime();

  starting_up = false;

  int name_len = strlen(name);

  static char num_str[max_len + 1];
  int num_len = 0;
  // TODO: update score
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
    case Surprise:
      strncpy(display_str, SURPRISE_NAME, SURPRISE_NAME_LEN);
      display_str[SURPRISE_NAME_LEN] = 0;
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

void ShowNameApp::SetScore(uint32_t score) {
  score_cache = score;
  update_display();
}

void ShowNameApp::SetSurpriseMsg(const char *msg) {
  int len = strlen(msg);
  if (len >= kDisplayScrollMaxTextLen) {
    len = kDisplayScrollMaxTextLen;
  }
  memcpy(surprise_msg, msg, len);
  surprise_msg[len] = 0;
}

enum ShowNameMode ShowNameApp::GetMode() { return mode; }

}  // namespace hitcon
