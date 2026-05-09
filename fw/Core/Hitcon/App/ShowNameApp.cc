#include "ShowNameApp.h"

#include <App/ConnectMenuApp.h>
#include <App/MainMenuApp.h>
#include <App/NameSettingApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/Display/font.h>
#include <Logic/GameScore.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>
#include <Util/uint_to_str.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::PeerType;

namespace hitcon {

namespace {

// Update once every 15s. Units: ms.
constexpr unsigned kMinUpdateInterval = 15 * 1000;
static const char SURPRISE_NAME[] = "You got pwned!";
static const int SURPRISE_NAME_LEN = sizeof(SURPRISE_NAME) / sizeof(char);
static constexpr unsigned SURPRISE_TIME = 10 * 1000;

}  // namespace
ShowNameApp show_name_app;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
ShowNameApp::ShowNameApp()
    : _routine_task(490, (task_callback_t)&ShowNameApp::check_update, this,
                    1000) {}
#pragma GCC diagnostic pop

void ShowNameApp::Init() {
  nv_storage_content &content = g_nv_storage.GetCurrentStorage();
  if (g_nv_storage.IsStorageValid() && strlen(content.name)) {
    memcpy(name, content.name, NAME_LEN);
  } else {
    strncpy(name, DEFAULT_NAME, NAME_LEN);
  }
  scheduler.Queue(&_routine_task, nullptr);
}

void ShowNameApp::OnEntry() {
  display_set_orientation(0);
  // TODO: update score with our new game
  CalculateScore();
  scheduler.EnablePeriodic(&_routine_task);
  starting_up = false;
  update_display();
}

void ShowNameApp::OnExit() {
  display_set_orientation(1);
  scheduler.DisablePeriodic(&_routine_task);
}

void ShowNameApp::OnButton(button_t button) {
  const PeerType peer = g_xboard_logic.GetPeer();
  switch (button) {
    case BUTTON_LONG_MODE:
      switch (peer) {
        case PeerType::Peer2025:
          badge_controller.change_app(&connect_menu);
          break;
        case PeerType::Legacy:
          badge_controller.change_app(&connect_legacy_menu);
          break;
        case PeerType::BaseStn2025:
          badge_controller.change_app(&connect_basestn_menu);
          break;
        default:
          badge_controller.change_app(&name_setting_menu);
          break;
      }
      break;

    case BUTTON_MODE:
      if (peer == PeerType::Peer2025) {
        badge_controller.change_app(&connect_menu);
      } else {
        badge_controller.change_app(&main_menu);
      }
      break;

    default:
      break;
  }
}

void ShowNameApp::CalculateScore() {
  // ScoreHistApp
  constexpr int max_display_score = 9999999;
  int total = g_game_score.GetScore(GameScoreType::GAME_DINO);
  if (total > max_display_score) total = max_display_score;
  total += g_game_score.GetScore(GameScoreType::GAME_SNAKE);
  if (total > max_display_score) total = max_display_score;
  total += g_game_score.GetScore(GameScoreType::GAME_SPACESHIP);
  if (total > max_display_score) total = max_display_score;
  total += g_game_score.GetScore(GameScoreType::GAME_TETRIS);
  if (total > max_display_score) total = max_display_score;
  score_cache = total;
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

  num_len = uint_to_chr(num_str, max_len + 1, score_);

  switch (mode) {
    case NameScore:
      if (name_len > max_len - num_len - 1) name_len = max_len - num_len - 1;
      memcpy(display_str, name, name_len);
      display_str[name_len] = '-';
      memcpy(display_str + name_len + 1, num_str, num_len);
      display_str[name_len + num_len + 1] = 0;
      break;
    case NameOnly:
      memcpy(display_str, name, name_len);
      display_str[name_len] = 0;
      break;
    case ScoreOnly:
      memcpy(display_str, num_str, num_len);
      display_str[num_len] = 0;
      break;
    case Surprise:
      memcpy(display_str, surprise_msg, strlen(surprise_msg) + 1);
      break;
    default:
      break;
  }
  display_set_mode_scroll_text(display_str);
}

void ShowNameApp::SetName(const char *name) {
  strncpy(this->name, name, NAME_LEN);
  nv_storage_content &content = g_nv_storage.GetCurrentStorage();
  memcpy(content.name, this->name, NAME_LEN + 1);
  g_nv_storage.MarkDirty();
  g_nv_storage.ForceFlush(nullptr, nullptr);
  update_display();
}

void ShowNameApp::SetMode(const enum ShowNameMode mode) {
  this->mode = mode;
  update_display();
}

void ShowNameApp::SetSurpriseMsg(const char *msg) {
  strncpy(surprise_msg, msg, kDisplayScrollMaxTextLen);
}

enum ShowNameMode ShowNameApp::GetMode() { return mode; }

}  // namespace hitcon
