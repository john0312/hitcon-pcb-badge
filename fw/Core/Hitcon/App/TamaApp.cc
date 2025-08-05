// #define FOR_TAMA_TEST

#include "TamaApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/NvStorage.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/Scheduler.h>

#include <cstdarg>
#include <cstring>

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
      _hatching_task(
          1000,
          (hitcon::service::sched::task_callback_t)&TamaApp::HatchingRoutine,
          this, 0),
      _tama_data(g_nv_storage.GetCurrentStorage().tama_storage),
      _current_selection_in_choose_mode(TAMA_TYPE::CAT), _fb() {}

void TamaApp::Init() {
#ifdef DEBUG
  // _tama_data is loaded from NvStorage.
  // If it's a fresh start (e.g., NvStorage is zeroed), _tama_data.type will be
  // 0 (NONE_TYPE).
  // use new data always for debugging
  _tama_data = {};
  g_nv_storage.MarkDirty();
#endif
  hitcon::service::sched::scheduler.Queue(&_routine_task, nullptr);
  // If the egg is hatching, enable background tasks for updating steps
  if (_tama_data.state == TAMA_APP_STATE::EGG_1 ||
      _tama_data.state == TAMA_APP_STATE::EGG_2 ||
      _tama_data.state == TAMA_APP_STATE::EGG_3 ||
      _tama_data.state == TAMA_APP_STATE::EGG_4) {
    _hatching_task.SetWakeTime(SysTimer::GetTime() + 5000);
    hitcon::service::sched::scheduler.Queue(&_hatching_task, nullptr);
  }
  qte.Init();
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
    memcpy(&_enemy_score, 0, sizeof(_enemy_score));
    if (_tama_data.hatched == false || _tama_data.hp == 0) {
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
  if (_tama_data.state == TAMA_APP_STATE::INTRO_TEXT) {
    // display_set_mode_scroll_text("Choose your pet");
  } else {
    if (_tama_data.hatched) {
      _tama_data.state = TAMA_APP_STATE::IDLE;
    }
    UpdateFrameBuffer();
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
  display_set_mode_fixed_packed(current_screen_buffer);
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
          _tama_data.state = TAMA_APP_STATE::EGG_1;
          _previous_hatching_step = g_imu_logic.GetStep();
          _hatching_task.SetWakeTime(SysTimer::GetTime() + 5000);
          scheduler.Queue(&_hatching_task, nullptr);
          needs_update_fb = true;
          needs_save = true;
          break;
#ifdef DEBUG
        case TAMA_APP_STATE::EGG_1:
          _tama_data.state = TAMA_APP_STATE::EGG_2;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_2:
          _tama_data.state = TAMA_APP_STATE::EGG_3;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_3:
          _tama_data.state = TAMA_APP_STATE::EGG_4;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_4:
          _tama_data.state = TAMA_APP_STATE::HATCHING;
          _total_hatchin_steps = 400;
          _frame_count = 0;
          needs_update_fb = true;
          break;
#endif
        case TAMA_APP_STATE::FEED_CONFIRM:
          if (_is_selected) {
            _tama_data.state = TAMA_APP_STATE::FEED_ANIME;
            needs_update_fb = true;
          } else {
            _tama_data.state = TAMA_APP_STATE::IDLE;
            needs_update_fb = true;
          }
          break;
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
        case TAMA_APP_STATE::IDLE:
          if (_tama_data.hp == 0) {
            break;
          }
          _tama_data.state = TAMA_APP_STATE::FEED_CONFIRM;
          needs_update_fb = true;
          break;
        // TODO: Handle other states for BUTTON_LEFT if necessary
        case TAMA_APP_STATE::FEED_CONFIRM:
          if (_is_selected == true) {
            _is_selected = false;
            needs_update_fb = true;
          }
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
        case TAMA_APP_STATE::IDLE:
          // block going to the next state if weak
          if (_tama_data.hp == 0) {
            break;
          }
          break;
          // TODO: Handle other states for BUTTON_LEFT if necessary
        case TAMA_APP_STATE::FEED_CONFIRM:
          if (_is_selected == false) {
            _is_selected = true;
            needs_update_fb = true;
          }
        default:
          break;
      }
      break;
    case BUTTON_UP:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::IDLE:
          _tama_data.state = TAMA_APP_STATE::LV_DETAIL;
          // needs_save = true;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::LV_DETAIL:
          _tama_data.state = TAMA_APP_STATE::IDLE;
          // needs_save = true;
          needs_update_fb = true;
          break;
      }
      break;
    case BUTTON_DOWN:
      switch (_tama_data.state) {
        case TAMA_APP_STATE::IDLE:
          _tama_data.state = TAMA_APP_STATE::LV_DETAIL;
          // needs_save = true;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::LV_DETAIL:
          _tama_data.state = TAMA_APP_STATE::IDLE;
          // needs_save = true;
          needs_update_fb = true;
          break;
      }
      break;
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

// void TamaApp::OnEdgeButton(button_t button) {}

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
      if (true) {
        // update some system value
        _tama_data.level = 1;
        _tama_data.hp = 3;
        // change type
        _tama_data.state = TAMA_APP_STATE::CHOOSE_TYPE;
        needs_render = true;
        needs_save = true;
        UpdateFrameBuffer();
      }
      break;
    case TAMA_APP_STATE::EGG_1:
    case TAMA_APP_STATE::EGG_2:
    case TAMA_APP_STATE::EGG_3:
    case TAMA_APP_STATE::EGG_4:
      needs_render = true;
      break;
    case TAMA_APP_STATE::HATCHING:
      if (_frame_count >= 10) {
        _tama_data.state = TAMA_APP_STATE::IDLE;
        _tama_data.hatched = true;
        needs_save = true;
        UpdateFrameBuffer();
      }
      needs_render = true;
      break;
    case TAMA_APP_STATE::IDLE:
      needs_render = true;
      break;
    case TAMA_APP_STATE::FEED_ANIME:
      if (_frame_count == 4) {
        _tama_data.state = TAMA_APP_STATE::PET_FED;
        UpdateFrameBuffer();
        break;
      }
      needs_render = true;
      break;
    case TAMA_APP_STATE::PET_FED:
      if (_frame_count == 6) {
        _tama_data.state = TAMA_APP_STATE::IDLE;
        UpdateFrameBuffer();
      }
    default:
      break;
  }

  if (needs_save) {
    g_nv_storage.MarkDirty();
  }

  if (needs_render) {
    Render();
  }
}
void TamaApp::UpdateFrameBuffer() {
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    XbUpdateFrameBuffer();
    return;
  }
  _frame_count = 0;
  switch (_tama_data.state) {
    case TAMA_APP_STATE::CHOOSE_TYPE:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(PET_SELECTION).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(PET_SELECTION), 0);
      if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
        StackOnFrame(&TAMA_COMPONENT_PET_SELECTION_CURSOR, 0);
      } else if (_current_selection_in_choose_mode == TAMA_TYPE::DOG) {
        StackOnFrame(&TAMA_COMPONENT_PET_SELECTION_CURSOR, 8);
      } else {
        my_assert(false);  // Should not happen if state is CHOOSE_TYPE
      }
      break;
    case TAMA_APP_STATE::EGG_1:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_1).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_1), 0);
      break;
    case TAMA_APP_STATE::EGG_2:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_2).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_2), 0);
      break;
    case TAMA_APP_STATE::EGG_3:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_3).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_3), 0);
      break;
    case TAMA_APP_STATE::EGG_4:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_4).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(EGG_4), 0);
      break;
    case TAMA_APP_STATE::HATCHING:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(HATCHING).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(HATCHING), 0);
      break;
    case TAMA_APP_STATE::IDLE: {
      const tama_ani_t* pet = nullptr;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        pet = _tama_data.hp ? &TAMA_GET_ANIMATION_DATA(DOG_IDLE)
                            : &TAMA_GET_ANIMATION_DATA(DOG_WEAK);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        pet = _tama_data.hp ? &TAMA_GET_ANIMATION_DATA(CAT_IDLE)
                            : &TAMA_GET_ANIMATION_DATA(CAT_WEAK);
      } else {
        my_assert(false);
      }
      my_assert(pet);
      switch (_tama_data.hp) {
        case 3:
          ConcateAnimtaions(2, pet, &TAMA_GET_ANIMATION_DATA(HEART_3));
          break;
        case 2:
          ConcateAnimtaions(2, pet, &TAMA_GET_ANIMATION_DATA(HEART_2));
          break;
        case 1:
          ConcateAnimtaions(2, pet, &TAMA_GET_ANIMATION_DATA(HEART_1));
          break;
        case 0:
          break;
        default:
          my_assert(false);
          break;
      }
      break;
    }
    case TAMA_APP_STATE::LV_DETAIL:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(LV).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(LV), 0);
      StackOnFrame(&TAMA_NUM_FONT[_tama_data.level / 100], 5);
      StackOnFrame(&TAMA_NUM_FONT[(_tama_data.level % 100) / 10], 9);
      StackOnFrame(&TAMA_NUM_FONT[_tama_data.level % 10], 13);
      // TODO: Add indicator for bonus level
      break;
    case TAMA_APP_STATE::FEED_CONFIRM:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(FEED_CONFIRM).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(FEED_CONFIRM), 0);
      if (_is_selected) {
        StackOnFrame(&TAMA_COMPONENT_Y_SELECTION_CURSOR, 13);
      } else {
        StackOnFrame(&TAMA_COMPONENT_N_SELECTION_CURSOR, 0);
      }
      break;
    case TAMA_APP_STATE::FEED_ANIME:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(FEEDING).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(FEEDING), 5);
      break;
    case TAMA_APP_STATE::PET_FED: {
      const tama_ani_t* pet = nullptr;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        pet = &TAMA_GET_ANIMATION_DATA(DOG_FED_HEALING);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        pet = &TAMA_GET_ANIMATION_DATA(CAT_FED_HEALING);
      } else {
        my_assert(false);  // Should not happen if state is CHOOSE_TYPE
      }
      TAMA_PREPARE_FB(_fb, pet->frame_count);
      TAMA_COPY_FB(_fb, *pet, 4);
      break;
    }
    case TAMA_APP_STATE::PET_HEALING: {
      const tama_ani_t* pet = nullptr;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        pet = &TAMA_GET_ANIMATION_DATA(DOG_FED_HEALING);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        pet = &TAMA_GET_ANIMATION_DATA(CAT_FED_HEALING);
      }
      TAMA_PREPARE_FB(_fb, pet->frame_count);
      TAMA_COPY_FB(_fb, *pet, 0);
      StackOnFrame(&TAMA_COMPONENT_HOSPITAL_ICONS, 8);
      break;
    }
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
      qte.OnButton(button);
      if (qte.IsDone()) {
        _my_nounce = g_fast_random_pool.GetRandom();
        tama_xboard_result_t result = {
            .packet_type = TAMA_XBOARD_PACKET_TYPE::PACKET_SCORE,
            .score = qte.GetScore(),
            .nonce = _my_nounce,
        };
        g_game_controller.SetBufferToUsername(result.user);
        g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&result),
                                      sizeof(result), TAMA_RECV_ID);
        display_set_mode_scroll_text("Waiting for enemy...");
        xboard_state = TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE;
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
      if (packet->len == sizeof(_enemy_score))
        memcpy(&_enemy_score, packet->data, sizeof(_enemy_score));
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
    qte.Entry();
    UpdateFrameBuffer();
  }
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE) {
    // Do nothing. The routine / rendering logic is handled by QTE
    // Return early to prevent rendering.
    return;
  }
  if (xboard_state == TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
    if (_enemy_state != TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
      return;
    }
    // We need to know enemy score to update our frames
    UpdateFrameBuffer();
    my_assert(_enemy_score.packet_type ==
              TAMA_XBOARD_PACKET_TYPE::PACKET_SCORE);
    // Send result with TwoBadgeActivity
    hitcon::game::TwoBadgeActivity activity = {
        .gameType = hitcon::game::EventType::kTama,
        .myScore = qte.GetScore(),
        .otherScore = _enemy_score.score,
        .nonce = _my_nounce + _enemy_score.nonce,
    };
    memcpy(activity.otherUser, _enemy_score.user, sizeof(_enemy_score.user));
    g_game_controller.SendTwoBadgeActivity(activity);
  }

  Render();
}

void TamaApp::TamaHeal() {
  // TODO: Display animation of restoring
  // self._tama_data.hp = ...
  Render();
}

void TamaApp::StackOnFrame(const tama_display_component_t* component,
                           int offset) {
  StackOnFrame(component, 0xff, offset);
}

void TamaApp::StackOnFrame(const tama_display_component_t* component,
                           display_buf_t mask, int offset) {
  for (int i = 0; i < _fb.fb_size; i++) {
    for (int j = offset; (j < component->length + offset && j < DISPLAY_WIDTH);
         j++) {
      _fb.fb[i][j] |= ((component->data)[j - offset] & mask);
    }
  }
}

void TamaApp::HatchingRoutine(void* unused) {
  if (_tama_data.state != TAMA_APP_STATE::EGG_1 &&
      _tama_data.state != TAMA_APP_STATE::EGG_2 &&
      _tama_data.state != TAMA_APP_STATE::EGG_3 &&
      _tama_data.state != TAMA_APP_STATE::EGG_4) {
    return;
  }
  unsigned int step = g_imu_logic.GetStep();
  constexpr int hatching_delta = TAMA_HATCHING_STEPS / 4;
  if (true) {
    _total_hatchin_steps += step - _previous_hatching_step;
    if (true) {
      _total_hatchin_steps = TAMA_HATCHING_STEPS;
      _tama_data.state = TAMA_APP_STATE::HATCHING;
      return;
    }
    switch (_total_hatchin_steps / hatching_delta) {
      case 0:
        _tama_data.state = TAMA_APP_STATE::EGG_1;
        break;
      case 1:
        _tama_data.state = TAMA_APP_STATE::EGG_2;
        break;
      case 2:
        _tama_data.state = TAMA_APP_STATE::EGG_3;
        break;
      case 3:
        _tama_data.state = TAMA_APP_STATE::EGG_4;
        break;
      default:
        my_assert(false);
        break;
    }
  }
  _previous_hatching_step = step;
  _hatching_task.SetWakeTime(SysTimer::GetTime() + 5000);
  scheduler.Queue(&_hatching_task, nullptr);
}

void TamaApp::ConcateAnimtaions(uint8_t count, ...) {
  // The animations passed should have the same frame count and should not
  // exceed the maxinum display length
  va_list args;
  va_start(args, count);
  uint8_t offset = 0;
  // The first animation is handled outside the loop to initialize _fb
  tama_ani_t* first = va_arg(args, tama_ani_t*);
  TAMA_PREPARE_FB(_fb,
                  first->frame_count);  // Initialize with the first
                                        // animation's frame count
  TAMA_COPY_FB(_fb, *first, 0);         // Copy the first animation
  offset += first->length;
  for (int i = 1; i < count; i++) {  // Start from the second animation
    tama_ani_t* next = va_arg(args, tama_ani_t*);
    my_assert(next->frame_count == _fb.fb_size);
    my_assert(offset + next->length <= DISPLAY_WIDTH);
    TAMA_COPY_FB(_fb, *next, offset);
    offset += next->length;
  }
  va_end(args);
}

void TamaQte::Routine() {
  if (state == kInGame) {
    if (game.IsDone()) {
      success += game.IsSuccess();
      if (++currentRound == 5) {
        Exit();
      } else {
        state = TamaQteState::kBetweenGame;
        nextGameStart = SysTimer::GetTime() + PAUSE_BETWEEN_QTE_GAMES;
      }
    }
    game.Update();
  } else if (state == kBetweenGame) {
    if (SysTimer::GetTime() - nextGameStart >= 0) {
      game.Init();
      state = TamaQteState::kInGame;
    }
  } else {
    return;
  }

  Render();
}

void TamaQte::Entry() {
  state = TamaQteState::kInGame;
  game.Init();
  currentRound = 0;
  scheduler.EnablePeriodic(&routineTask);
}

void TamaQte::Exit() {
  state = TamaQteState::kDone;
  scheduler.DisablePeriodic(&routineTask);
}

void TamaQte::Init() {
  state = TamaQteState::kDone;
  scheduler.Queue(&routineTask, nullptr);
}

bool TamaQte::IsDone() { return state == TamaQteState::kDone; }

uint8_t TamaQte::GetScore() { return success * success; }

TamaQte::TamaQte()
    : routineTask(650, (callback_t)&TamaQte::Routine, this, QTE_REFRESH_RATE) {}

void TamaQte::OnButton(button_t button) {
  if (state != kInGame) return;
  if (game.IsDone()) return;
  if (button != BUTTON_OK) return;
  game.OnButton();
}

void TamaQte::Render() {
  display_buf_t fb[DISPLAY_WIDTH] = {0};
  game.Render(fb);
  display_set_mode_fixed_packed(fb);
}

void TamaQteGame::Init() {
  arrow.Init();
  target.Init();
  done = false;
  success = false;
}

void TamaQteGame::Render(display_buf_t* buf) {
  arrow.Render(buf);
  target.Render(buf);
}

void TamaQteGame::Update() {
  if (done) return;
  arrow.Update();
  if (arrow.IsDone()) {
    done = true;
  }
}

void TamaQteGame::OnButton() {
  if (done) return;
  if (arrow.IsDone()) return;
  done = true;
  success = arrow.GetLocation() == target.GetLocation();
}

bool TamaQteGame::IsSuccess() { return !done && success; }

bool TamaQteGame::IsDone() { return done; }

void TamaQteArrow::Init() {
  location = 0;
  direction = 1;
  done = false;
}

void TamaQteArrow::Update() {
  if (done) return;
  location += direction;
  if (location == DISPLAY_WIDTH - 1) direction *= -1;
  if (location == -1 && direction == -1) {
    location = 0;
    done = true;
  }
}

void TamaQteArrow::Render(display_buf_t* buf) {
  buf[location] |= 0b11111100;
  if (location - 1 >= 0) buf[location - 1] |= 0b00001000;
  if (location - 2 >= 0) buf[location - 2] |= 0b00010000;
  if (location < DISPLAY_WIDTH - 1) buf[location + 1] |= 0b00001000;
  if (location < DISPLAY_WIDTH - 2) buf[location + 2] |= 0b00010000;
}

bool TamaQteArrow::IsDone() { return done; }

uint8_t TamaQteArrow::GetLocation() { return location; }

void TamaQteTarget::Render(display_buf_t* buf) { buf[location] |= 0b00001111; }

uint8_t TamaQteTarget::GetLocation() { return location; }

void TamaQteTarget::Init() {
  location = g_fast_random_pool.GetRandom() % 15 + 1;
}

}  // namespace tama
}  // namespace app
}  // namespace hitcon
