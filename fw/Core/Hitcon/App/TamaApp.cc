// #define FOR_TAMA_TEST

#include "TamaApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/NvStorage.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>

#include <cstring>  // For memset if needed, though direct assignment is used

#include "tama_src/TamaAppFrame.h"

using namespace hitcon::service::xboard;
using hitcon::service::sched::my_assert;

namespace hitcon {
namespace app {
namespace tama {

TamaApp tama_app;

TamaApp::TamaApp()
    : _routine_task(930,  // Task priority
                    (hitcon::service::sched::task_callback_t)&TamaApp::Routine,
                    (void*)this, ROUTINE_INTERVAL_MS),
      _tama_data(g_nv_storage.GetCurrentStorage().tama_storage),
      _current_selection_in_choose_mode(TAMA_TYPE::CAT), _fb() {}

void TamaApp::Init() {
  hitcon::service::sched::scheduler.Queue(&_routine_task, nullptr);
// _tama_data is loaded from NvStorage.
// If it's a fresh start (e.g., NvStorage is zeroed), _tama_data.type will be
// 0 (NONE_TYPE).
// use new data always for debugging
#ifdef DEBUG
  _tama_data = {};
#endif
  g_nv_storage.MarkDirty();
}

void SetSingleplayer() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_SINGLEPLAYER;
  tama_app.xboard_state = TAMA_XBOARD_STATE::XBOARD_INVITE;
  tama_app.xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N;
}

void SetMultiplayer() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_MULTIPLAYER;
}

void SetBaseStationConnect() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_BASESTATION;
}

void TamaApp::OnEntry() {
  hitcon::service::sched::scheduler.EnablePeriodic(&_routine_task);
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    g_xboard_logic.SetOnPacketArrive((callback_t)&TamaApp::OnXBoardRecv, this,
                                     TAMA_RECV_ID);
    _enemy_state = TAMA_XBOARD_STATE::XBOARD_INVITE;
    _enemy_score = nullptr;
    if (_tama_data.state != TAMA_APP_STATE::ALIVE) {
      xboard_state = TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE;
      display_set_mode_scroll_text("Your pet is not ready yet");
      TAMA_XBOARD_PACKET_TYPE packet =
          TAMA_XBOARD_PACKET_TYPE::PACKET_UNAVAILABLE;
      g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&packet),
                                    sizeof(packet), TAMA_RECV_ID);
    }
    return;
  }
  if (player_mode == TAMA_PLAYER_MODE::MODE_BASESTATION) {
    TamaHeal();
    return;
  }
  my_assert(player_mode == TAMA_PLAYER_MODE::MODE_SINGLEPLAYER);
  if (_tama_data.state == TAMA_APP_STATE::CHOOSE_TYPE) {
    // Ensure _current_selection_in_choose_mode is valid. Default to DOG if type
    // is NONE.
  } else if (_tama_data.state == TAMA_APP_STATE::INTRO_TEXT) {
    // If OnEntry is called while still in INTRO_TEXT, ensure scrolling text is
    // set.
    display_set_mode_scroll_text("Choose your pet");
  }
}

void TamaApp::OnExit() {
  hitcon::service::sched::scheduler.DisablePeriodic(&_routine_task);
  // NvStorage will be flushed by the system if MarkDirty was called.
}

void TamaApp::Render() {
  if (
      // INTRO_TEXT handles render in display_set_mode_scroll_text
      _tama_data.state == TAMA_APP_STATE::INTRO_TEXT) {
    return;
  }

  // Ensure fb_size is valid to prevent division by zero.
  // The constructor should have initialized this.
  if (_fb.fb_size == 0) {
    return;
  }

  display_buf_t* current_screen_buffer = _fb.fb[_fb.active_frame];
  // render non-compressed data
  display_set_mode_fixed(current_screen_buffer);
  _fb.active_frame = (_fb.active_frame + 1) % _fb.fb_size;
  _frame_count++;
}

void TamaApp::OnButton(button_t button) {
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    XbOnButton(button);
    return;
  }
  bool needs_save = false;
  bool needs_update_fb = false;

  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      return;  // Exit immediately

    case BUTTON_OK:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::INTRO_TEXT:
          // just wait for the text to scroll finished
          break;
        case TAMA_APP_STATE::CHOOSE_TYPE:
          _tama_data.type = _current_selection_in_choose_mode;
          _tama_data.state = TAMA_APP_STATE::EGG;
          _tama_data.hatching_start_shaking_count = g_imu_logic.GetStep();
          needs_update_fb = true;
          needs_save = true;
          break;
#ifdef DEBUG
        case TAMA_APP_STATE::EGG:
          // Hatch egg for debug use
          hatching_warning_frame_count = 0;
#endif
        default:
          // No action for other states on OK press, or handle as needed
          break;
      }
      break;
    case BUTTON_LEFT:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::CHOOSE_TYPE:
          if (_current_selection_in_choose_mode == TAMA_TYPE::DOG) {
            needs_update_fb = true;
          }
          _current_selection_in_choose_mode = TAMA_TYPE::CAT;
          break;
          // TODO: Handle other states for BUTTON_LEFT if necessary
        default:
          break;
      }
      break;
    case BUTTON_RIGHT:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::CHOOSE_TYPE:
          if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
            needs_update_fb = true;
          }
          _current_selection_in_choose_mode = TAMA_TYPE::DOG;
          break;
          // TODO: Handle other states for BUTTON_LEFT if necessary

#ifdef FOR_TAMA_TEST
        case TAMA_APP_STATE::EGG:
          _tama_data.latest_shaking_count += 50;
          needs_save = true;
          needs_update_fb = true;
          break;
#endif
        default:
          break;
      }
    default:
      break;
  }

  if (needs_save) {
    g_nv_storage.MarkDirty();
  }
  if (needs_update_fb) {
    UpdateFrameBuffer();
    Render();
  }
}

void TamaApp::OnEdgeButton(button_t button) {}

void TamaApp::Routine(void* unused) {
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    XbRoutine(unused);
    return;
  }
  if (player_mode == TAMA_PLAYER_MODE::MODE_BASESTATION) {
    // Do not do anything for healing in routine. This is so simple so
    // we just do it in TamaHeal which is called from OnEntry.
    return;
  }
  my_assert(player_mode == TAMA_PLAYER_MODE::MODE_SINGLEPLAYER);
  bool needs_render = true;
  bool needs_save = false;

  switch (_tama_data.state) {
    case TAMA_APP_STATE::INTRO_TEXT:
      needs_render = false;
      if (display_get_scroll_count() >= 1) {
        _tama_data.state = TAMA_APP_STATE::CHOOSE_TYPE;
        needs_render = true;
        UpdateFrameBuffer();
      }
      break;
    case TAMA_APP_STATE::EGG:
      int latest_shaking_count;
      latest_shaking_count = g_imu_logic.GetStep();

      if (_tama_data.latest_shaking_count != latest_shaking_count) {
        _tama_data.latest_shaking_count = latest_shaking_count;
        needs_save = true;
        needs_render = true;
        UpdateFrameBuffer();
      }
      break;
    case TAMA_APP_STATE::HATCHING:
      if (hatching_warning_frame_count < 0) {
        _tama_data.state = TAMA_APP_STATE::ALIVE;
        needs_save = true;
      }
      hatching_warning_frame_count--;
      needs_render = true;
      UpdateFrameBuffer();
      break;
    case TAMA_APP_STATE::ALIVE:
      if (anime_frame == 0) {
        anime_frame = 1;
      } else if (anime_frame == 1) {
        anime_frame = 0;
      }
      needs_render = true;
      UpdateFrameBuffer();
      break;
    default:
      break;
  }

  if (needs_save) {
    g_nv_storage.MarkDirty();
  }

  if (needs_render) {
    // Only render if this app is currently active.
    // This check might be redundant if BadgeController ensures OnEntry/OnExit
    // are paired correctly with task enabling/disabling, but it's a safe check.
    if (hitcon::badge_controller.GetCurrentApp() == this) {
      Render();
    }
  }
}
void TamaApp::UpdateFrameBuffer() {
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    XbUpdateFrameBuffer();
    return;
  }
  uint8_t frame[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {0};

  switch (_tama_data.state) {
    case TAMA_APP_STATE::CHOOSE_TYPE:
      if (_current_selection_in_choose_mode == TAMA_TYPE::DOG) {
        get_select_character_frame(RIGHT, frame);
      } else if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
        get_select_character_frame(LEFT, frame);
      } else {
        my_assert(false);  // Should not happen if state is CHOOSE_TYPE
      }

      _fb.fb_size = 1;
      memcpy(_fb.fb[0], frame,
             sizeof(display_buf_t[DISPLAY_HEIGHT * DISPLAY_WIDTH]));

      break;
    case TAMA_APP_STATE::EGG:
      int remaining_count;
      remaining_count =
          HATCH_START_COUNT - (_tama_data.latest_shaking_count -
                               _tama_data.hatching_start_shaking_count);
      if (remaining_count < 0) {
        _tama_data.state = TAMA_APP_STATE::HATCHING;
      } else {
        get_hatch_status_frame(remaining_count, frame);
        _fb.fb_size = 1;
        memcpy(_fb.fb[0], frame,
               sizeof(display_buf_t[DISPLAY_HEIGHT * DISPLAY_WIDTH]));
      }
      break;
    case TAMA_APP_STATE::HATCHING:
      _fb.fb_size = 1;
      get_hatch_born_warning_frame(hatching_warning_frame_count % 2, frame);
      memcpy(_fb.fb[0], frame,
             sizeof(display_buf_t[DISPLAY_HEIGHT * DISPLAY_WIDTH]));

      break;
    case TAMA_APP_STATE::ALIVE:
      _fb.fb_size = 1;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        get_dog_idle_frame_with_status_overview(anime_frame, 3, 4, frame);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        get_cat_idle_frame_with_status_overview(anime_frame, 3, 4, frame);
      } else {
        my_assert(false);  // Should not happen in ALIVE state
      }
      memcpy(_fb.fb[0], frame,
             sizeof(display_buf_t[DISPLAY_HEIGHT * DISPLAY_WIDTH]));
      break;
    default:
      // Should not happen in CHOOSE_TYPE state
      my_assert(false);
      break;
      _frame_count = 0;
  }
}

void TamaApp::XbOnButton(button_t button) {
  // TODO: Handle all XBoard button here
  switch (xboard_state) {
    case TAMA_XBOARD_STATE::XBOARD_INVITE:
      switch (button & BUTTON_VALUE_MASK) {
        case BUTTON_OK: {
          uint8_t invite;
          if (xboard_battle_invite ==
              TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N) {
            invite =
                static_cast<uint8_t>(TAMA_XBOARD_PACKET_TYPE::PACKET_CONFIRM);
          } else {
            my_assert(xboard_battle_invite ==
                      TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_Y);
            invite =
                static_cast<uint8_t>(TAMA_XBOARD_PACKET_TYPE::PACKET_LEAVE);
          }
          g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&invite),
                                        sizeof(invite), TAMA_RECV_ID);
          xboard_state = TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER;
          display_set_mode_scroll_text("Waiting for enemy...");
          UpdateFrameBuffer();
          break;
        }
        case BUTTON_LEFT:
          xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N;
          UpdateFrameBuffer();
          break;
        case BUTTON_RIGHT:
          xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_Y;
          UpdateFrameBuffer();
          break;
      }
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER:
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE:
      if (button & BUTTON_VALUE_MASK == BUTTON_OK) {
        // TODO: get score
        _qte_count++;
        if (_qte_count == 5) {
          _my_nounce = g_fast_random_pool.GetRandom();
          tama_xboard_result_t result = {
              .packet_type = TAMA_XBOARD_PACKET_TYPE::PACKET_SCORE,
              .score = _qte_score,
              .nonce = _my_nounce,
          };
          g_game_controller.GetUsername(result.user);
          g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&result),
                                        sizeof(result), TAMA_RECV_ID);
          display_set_mode_scroll_text("Waiting for enemy...");
        }
      }
      break;
    case TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE:
      break;
    default:
      my_assert(false);
      break;
  }
}
void TamaApp::XbUpdateFrameBuffer() {
  // TODO: Handle all XBoard frame here
  switch (xboard_state) {
    case TAMA_XBOARD_STATE::XBOARD_INVITE:
      // TODO: Draw frame buffer for battel invite
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER:
      // TODO: Draw frame buffer for battle encounter
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE:
      // TODO: Draw frame buffer for QTE
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE:
      // TODO: Draw frame buffer for sent score
      my_assert(_enemy_score);
      break;
    default:
      my_assert(false);
      break;
  }
}

void TamaApp::OnXBoardRecv(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  switch ((TAMA_XBOARD_PACKET_TYPE)packet->data[0]) {
    // TODO: Handle XB game logic here
    case TAMA_XBOARD_PACKET_TYPE::PACKET_CONFIRM:
      _enemy_state = TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER;
      break;
    case TAMA_XBOARD_PACKET_TYPE::PACKET_SCORE:
      _enemy_score = reinterpret_cast<tama_xboard_result_t*>(packet->data);
      break;
    case TAMA_XBOARD_PACKET_TYPE::PACKET_END:
      // TODO: End game
      break;
    case TAMA_XBOARD_PACKET_TYPE::PACKET_LEAVE:
      badge_controller.BackToMenu(this);
      return;  // Exit immediately
    case TAMA_XBOARD_PACKET_TYPE::PACKET_UNAVAILABLE:
      _enemy_state = TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE;
      display_set_mode_scroll_text("Enemy unavailable");
      break;
    default:
      my_assert(false);
      break;
  }
}

void TamaApp::XbRoutine(void* unused) {
  // TODO: Handle all XBoard routine here
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE ||
      _enemy_state == TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE) {
    // We can not battle now. Do nothing to let the display scroll
    return;
  }

  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
      !(_enemy_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER)) {
    return;
  }
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
      _frame_count >= 8) {
    xboard_state = TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE;
    _qte_count = 0;
    _qte_score = 0;
    UpdateFrameBuffer();
  }
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE) {
    // TODO: implement QTE game logic here
  }
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
    if (_enemy_state != TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
      return;
    }
    // We need to know enemy score to update our frames
    UpdateFrameBuffer();
    my_assert(_enemy_score);
    my_assert(_enemy_score->packet_type ==
              TAMA_XBOARD_PACKET_TYPE::PACKET_SCORE);
    // Send result with TwoBadgeActivity
    hitcon::game::TwoBadgeActivity activity = {
        .gameType = hitcon::game::EventType::kTama,
        .myScore = _qte_score,
        .otherScore = _enemy_score->score,
        .nonce = _my_nounce + _enemy_score->nonce,
    };
    memcpy(activity.otherUser, _enemy_score->user, sizeof(_enemy_score->user));
    g_game_controller.SendTwoBadgeActivity(activity);
  }

  Render();
}

void TamaApp::TamaHeal() {
  // TODO: Display animation of restoring
  // self._tama_data.hp = ...
  Render();
}

}  // namespace tama
}  // namespace app
}  // namespace hitcon
