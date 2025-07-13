#include "TamaApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/NvStorage.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>

#include <cstring>  // For memset if needed, though direct assignment is used

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
      _current_selection_in_choose_mode(TAMA_TYPE::DOG), _fb() {}

void TamaApp::Init() {
  hitcon::service::sched::scheduler.Queue(&_routine_task, nullptr);
  // _tama_data is loaded from NvStorage.
  // If it's a fresh start (e.g., NvStorage is zeroed), _tama_data.type will be
  // 0 (NONE_TYPE).
  // use new data always for debugging
  _tama_data = {};
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

void TamaApp::OnEntry() {
  hitcon::service::sched::scheduler.EnablePeriodic(&_routine_task);
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    g_xboard_logic.SetOnPacketArrive((callback_t)&TamaApp::OnXBoardRecv, this,
                                     TAMA_RECV_ID);
    return;
  }
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
      _tama_data.state == TAMA_APP_STATE::INTRO_TEXT ||
      // XBoard Waiting user input
      (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
       !(_enemy_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER))) {
    return;
  }

  // Ensure fb_size is valid to prevent division by zero.
  // The constructor should have initialized this.
  if (_fb.fb_size == 0) {
    return;
  }

  display_buf_t* current_screen_buffer = _fb.fb[_fb.active_frame];
  display_set_mode_fixed_packed(current_screen_buffer);
  _fb.active_frame = (_fb.active_frame + 1) % _fb.fb_size;
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
          break;
        case TAMA_APP_STATE::CHOOSE_TYPE:
          _tama_data.type = _current_selection_in_choose_mode;
          _tama_data.state = TAMA_APP_STATE::EGG;
          needs_update_fb = true;
          needs_save = true;
          break;
        case TAMA_APP_STATE::EGG:
          // This allows pressing OK on the egg to hatch it immediately.
          _tama_data.state = TAMA_APP_STATE::ALIVE;
          needs_update_fb = true;
          needs_save = true;
          break;
        default:
          // No action for other states on OK press, or handle as needed
          break;
      }
      break;
    case BUTTON_LEFT:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::CHOOSE_TYPE:
          if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
            needs_update_fb = true;
          }
          _current_selection_in_choose_mode = TAMA_TYPE::DOG;
          break;
          // TODO: Handle other states for BUTTON_LEFT if necessary
        default:
          break;
      }
      break;
    case BUTTON_RIGHT:
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
  switch (_tama_data.state) {
    case TAMA_APP_STATE::CHOOSE_TYPE:
      const tama_ani_t* selected_animation;
      if (_current_selection_in_choose_mode == TAMA_TYPE::DOG) {
        selected_animation =
            &animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::DOG)];
      } else if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
        selected_animation =
            &animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::CAT)];
      } else {
        my_assert(false);  // Should not happen if state is CHOOSE_TYPE
      }

      _fb.fb_size = selected_animation->frame_count;
      _fb.active_frame = 0;  // Start from the first frame
      switch (_current_selection_in_choose_mode) {
        case TAMA_TYPE::DOG:
          // Copy DOG animation frames to left
          for (int i = 0; i < _fb.fb_size; ++i) {
            memset(_fb.fb[i], 0, sizeof(display_buf_t) * DISPLAY_WIDTH);
            memcpy(_fb.fb[i], selected_animation->frames_data[i],
                   sizeof(display_buf_t[8]));
            // Draw right arrow
            for (int col = 0; col < BITMAP_RIGHT_ARROW_WIDTH; ++col) {
              _fb.fb[i][DISPLAY_WIDTH - 1 - col] |=
                  bitmap_right_arrow_cols[BITMAP_RIGHT_ARROW_WIDTH - 1 - col];
            }
          }
          break;
        case TAMA_TYPE::CAT:
          // Copy CAT animation frames to right
          for (int i = 0; i < _fb.fb_size; ++i) {
            memset(_fb.fb[i], 0, sizeof(display_buf_t) * DISPLAY_WIDTH);
            memcpy(_fb.fb[i] + (DISPLAY_WIDTH - sizeof(display_buf_t[8])),
                   selected_animation->frames_data[i],
                   sizeof(display_buf_t[8]));
            // Draw left arrow
            for (int col = 0; col < BITMAP_LEFT_ARROW_WIDTH; ++col) {
              _fb.fb[i][col] |= bitmap_left_arrow_cols[col];
            }
          }
      }
      break;
    case TAMA_APP_STATE::EGG:
      // Draw egg animation in the cneter
      selected_animation =
          &animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::EGG)];
      _fb.fb_size = selected_animation->frame_count;
      _fb.active_frame = 0;  // Start from the first frame
      for (int i = 0; i < _fb.fb_size; ++i) {
        // Center the egg animation (8 columns wide)
        memset(_fb.fb[i], 0, sizeof(display_buf_t) * DISPLAY_WIDTH);
        memcpy(_fb.fb[i] + (DISPLAY_WIDTH / 2 - 4), TAMA_EGG_FRAMES[i],
               sizeof(display_buf_t[8]));
      }
      break;
    case TAMA_APP_STATE::ALIVE:
      // Draw pet animation in the center
      if (_tama_data.type == TAMA_TYPE::DOG) {
        selected_animation =
            &animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::DOG)];
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        selected_animation =
            &animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::CAT)];
      } else {
        // Should not happen if state is ALIVE
        my_assert(false);
      }
      _fb.fb_size = selected_animation->frame_count;
      _fb.active_frame = 0;  // Start from the first frame
      for (int i = 0; i < _fb.fb_size; ++i) {
        // Center the pet animation (8 columns wide)
        memset(_fb.fb[i], 0, sizeof(display_buf_t) * DISPLAY_WIDTH);
        memcpy(_fb.fb[i] + (DISPLAY_WIDTH / 2 - 4),
               selected_animation->frames_data[i], sizeof(display_buf_t[8]));
      }
      break;
    default:
      // Should not happen in CHOOSE_TYPE state
      my_assert(false);
      break;
  }
}

void TamaApp::XbOnButton(button_t button) {
  // TODO: Handle all XBoard button here
  switch (xboard_state) {
    case TAMA_XBOARD_STATE::XBOARD_INVITE:
      switch (button & BUTTON_VALUE_MASK) {
        case BUTTON_OK: {
          TAMA_XBOARD_BATTLE_INVITE invite = xboard_battle_invite;
          g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&invite),
                                        sizeof(invite), TAMA_RECV_ID);
          xboard_state = TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER;
          display_set_mode_scroll_text("Waiting for enemy...");
          XbUpdateFrameBuffer();
          break;
        }
        case BUTTON_LEFT:
          xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N;
          XbUpdateFrameBuffer();
          break;
        case BUTTON_RIGHT:
          xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_Y;
          XbUpdateFrameBuffer();
          break;
      }
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER:
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
      // TODO: Update score
      break;
    case TAMA_XBOARD_PACKET_TYPE::PACKET_END:
      // TODO: End game
      break;
    case TAMA_XBOARD_PACKET_TYPE::PACKET_LEAVE:
      badge_controller.BackToMenu(this);
      return;  // Exit immediately
    default:
      my_assert(false);
      break;
  }
}

}  // namespace tama
}  // namespace app
}  // namespace hitcon
