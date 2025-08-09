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
      _hunger_task(
          600, (hitcon::service::sched::task_callback_t)&TamaApp::HungerRoutine,
          this, 30000),
      _level_up_task(
          600,
          (hitcon::service::sched::task_callback_t)&TamaApp::LevelUpRoutine,
          this, 300000),
      _tama_data(g_nv_storage.GetCurrentStorage().tama_storage),
      _state(_tama_data.state),
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
  if (_state == TAMA_APP_STATE::EGG_1 || _state == TAMA_APP_STATE::EGG_2 ||
      _state == TAMA_APP_STATE::EGG_3 || _state == TAMA_APP_STATE::EGG_4) {
    _hatching_task.SetWakeTime(SysTimer::GetTime() + 5000);
    hitcon::service::sched::scheduler.Queue(&_hatching_task, nullptr);
  }
  hitcon::service::sched::scheduler.Queue(&_hunger_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&_hunger_task);
  qte.Init();
}

void SetSingleplayer() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_SINGLEPLAYER;
}

void SetMultiplayer() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_MULTIPLAYER;

  tama_app.my_packet = {.state = TAMA_XBOARD_STATE::XBOARD_INVITE};
  tama_app.enemy_packet = {.state = TAMA_XBOARD_STATE::XBOARD_INVITE};
  tama_app.xboard_battle_invite = TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N;
}

void SetBaseStationConnect() {
  tama_app.player_mode = TAMA_PLAYER_MODE::MODE_BASESTATION;
}

void TamaApp::OnEntry() {
  hitcon::service::sched::scheduler.EnablePeriodic(&_routine_task);
  if (player_mode == TAMA_PLAYER_MODE::MODE_MULTIPLAYER) {
    enemy_packet = {.state = TAMA_XBOARD_STATE::XBOARD_INVITE};
    g_xboard_logic.SetOnPacketArrive((callback_t)&TamaApp::OnXBoardRecv, this,
                                     TAMA_RECV_ID);
    if (_tama_data.state != TAMA_APP_STATE::IDLE || _tama_data.hp == 0) {
      my_packet.state = TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE;
      display_set_mode_scroll_text("Your pet is not ready yet");
    } else {
      my_packet = {.state = TAMA_XBOARD_STATE::XBOARD_INVITE};
    }
    UpdateFrameBuffer();
    return;
  }
  if (player_mode == TAMA_PLAYER_MODE::MODE_BASESTATION) {
    TamaHeal();
    return;
  }
  my_assert(player_mode == TAMA_PLAYER_MODE::MODE_SINGLEPLAYER);
  _state = _tama_data.state;
  if (_state == TAMA_APP_STATE::INTRO_TEXT) {
    display_set_mode_scroll_text("Choose your pet");
  } else {
    UpdateFrameBuffer();
  }
}

void TamaApp::OnExit() {
  hitcon::service::sched::scheduler.DisablePeriodic(&_routine_task);
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE) {
    // We are aborting the app during QTE battle. Abort the QTE.
    qte.Exit();
  }
  // NvStorage will be flushed by the system if MarkDirty was called.
}

void TamaApp::Render() {
  if (
      // INTRO_TEXT handles render in display_set_mode_scroll_text
      _state == TAMA_APP_STATE::INTRO_TEXT) {
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
  if (_state == TAMA_APP_STATE::TRAINING_QTE) {
    qte.OnButton(button);
    return;
  }
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      return;  // Exit immediately

    case BUTTON_OK:
      switch (_state) {
        case TAMA_APP_STATE::INTRO_TEXT:
          // just wait for the text to scroll finished
          _state = TAMA_APP_STATE::CHOOSE_TYPE;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::CHOOSE_TYPE:
          _tama_data.type = _current_selection_in_choose_mode;
          _state = TAMA_APP_STATE::EGG_1;
          _previous_hatching_step = g_imu_logic.GetStep();
          _hatching_task.SetWakeTime(SysTimer::GetTime() + 5000);
          scheduler.Queue(&_hatching_task, nullptr);
          needs_update_fb = true;
          needs_save = true;
          break;
#ifdef DEBUG
        case TAMA_APP_STATE::EGG_1:
          _state = TAMA_APP_STATE::EGG_2;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_2:
          _state = TAMA_APP_STATE::EGG_3;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_3:
          _state = TAMA_APP_STATE::EGG_4;
          _total_hatchin_steps += 100;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::EGG_4:
          _state = TAMA_APP_STATE::HATCHING;
          _total_hatchin_steps = 400;
          _frame_count = 0;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::IDLE:
          _tama_data.hp = _tama_data.hp ? _tama_data.hp - 1 : 3;
          if (_tama_data.hunger == 0) SetHunger(4);
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::LV_DETAIL:
          if (_tama_data.secret_level < TAMA_MAX_SECRET_LEVEL) {
            _tama_data.secret_level++;
          } else {
            _tama_data.secret_level = 0;
          }
          needs_update_fb = true;
          needs_save = true;
          break;
#endif
        case TAMA_APP_STATE::FEED_CONFIRM:
          _state =
              _is_selected ? TAMA_APP_STATE::FEED_ANIME : TAMA_APP_STATE::IDLE;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::TRAINING_CONFIRM:
          _state =
              _is_selected ? TAMA_APP_STATE::TRAINING : TAMA_APP_STATE::IDLE;
          needs_update_fb = true;
          break;
        default:
          // No action for other states on OK press, or handle as needed
          break;
      }
      break;
    case BUTTON_LEFT:
      switch (_state) {
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
          _state = TAMA_APP_STATE::FEED_CONFIRM;
          needs_update_fb = true;
          break;
        // TODO: Handle other states for BUTTON_LEFT if necessary
        case TAMA_APP_STATE::FEED_CONFIRM:
          if (_is_selected == true) {
            _is_selected = false;
            needs_update_fb = true;
          }
          break;
        case TAMA_APP_STATE::TRAINING_CONFIRM:
          if (_is_selected == true) {
            _is_selected = false;
            needs_update_fb = true;
          }
          break;
        default:
          break;
      }
      break;
    case BUTTON_RIGHT:
      switch (_state) {
        case TAMA_APP_STATE::CHOOSE_TYPE:
          if (_current_selection_in_choose_mode == TAMA_TYPE::CAT) {
            needs_update_fb = true;
          }
          _current_selection_in_choose_mode = TAMA_TYPE::DOG;
          break;
        case TAMA_APP_STATE::IDLE:
          // block going to the next state if weak
          if (_tama_data.hp != 0) {
            _state = TAMA_APP_STATE::TRAINING_CONFIRM;
            needs_update_fb = true;
          }
          break;
        case TAMA_APP_STATE::FEED_CONFIRM:
          if (_is_selected == false) {
            _is_selected = true;
            needs_update_fb = true;
          }
          break;
        case TAMA_APP_STATE::TRAINING_CONFIRM:
          if (_is_selected == false) {
            _is_selected = true;
            needs_update_fb = true;
          }
          break;
        default:
          break;
      }
      break;
    case BUTTON_UP:
      switch (_state) {
        case TAMA_APP_STATE::IDLE:
          _state = TAMA_APP_STATE::LV_DETAIL;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::LV_DETAIL:
          _state = TAMA_APP_STATE::IDLE;
          needs_update_fb = true;
          break;
      }
      break;
    case BUTTON_DOWN:
      switch (_state) {
        case TAMA_APP_STATE::IDLE:
          _state = TAMA_APP_STATE::LV_DETAIL;
          needs_update_fb = true;
          break;
        case TAMA_APP_STATE::LV_DETAIL:
          _state = TAMA_APP_STATE::IDLE;
          needs_update_fb = true;
          break;
      }
      break;
    default:
      break;
  }

  if (needs_save && (_state != _tama_data.state)) {
    _tama_data.state = _state;
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

  // If the player is connected to the base station, behavior should be the same
  // as if it's not connected.
  my_assert(player_mode == TAMA_PLAYER_MODE::MODE_SINGLEPLAYER ||
            player_mode == TAMA_PLAYER_MODE::MODE_BASESTATION);
  bool needs_render = false;
  bool needs_save = false;

  switch (_state) {
    case TAMA_APP_STATE::INTRO_TEXT:
      needs_render = false;
      if (display_get_scroll_count() >= 1) {
        // change type
        _state = TAMA_APP_STATE::CHOOSE_TYPE;
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
      if (_frame_count >= 8) {
        _state = TAMA_APP_STATE::IDLE;
        _tama_data.qte_level = 1;
        _tama_data.hp = 3;
        SetHunger(4);
        needs_save = true;
        UpdateFrameBuffer();
      }
      needs_render = true;
      break;
    case TAMA_APP_STATE::IDLE:
      needs_render = true;
      break;
    case TAMA_APP_STATE::FEED_ANIME:
      if (_frame_count == TAMA_GET_ANIMATION_DATA(FEEDING).frame_count) {
        SetHunger(_tama_data.hunger + 1);
        needs_save = true;
        _state = TAMA_APP_STATE::PET_FED;
        UpdateFrameBuffer();
        break;
      }
      needs_render = true;
      break;
    case TAMA_APP_STATE::PET_FED:
      if (_frame_count == 6) {
        _state = TAMA_APP_STATE::IDLE;
        UpdateFrameBuffer();
      }
    case TAMA_APP_STATE::LV_DETAIL:
    case TAMA_APP_STATE::FEED_CONFIRM:
    case TAMA_APP_STATE::TRAINING_CONFIRM:
      needs_render = true;
      break;
    case TAMA_APP_STATE::TRAINING:
      if (_frame_count > 8) {
        _state = TAMA_APP_STATE::TRAINING_QTE;
        qte.Entry();
        return;
      }
      needs_render = true;
      break;
    case TAMA_APP_STATE::TRAINING_QTE:
      if (qte.IsDone()) {
        _state = TAMA_APP_STATE::TRAINING_END;
        if (qte.GetSuccess() == QTE_TOTAL_ROUNDS) {
          SetQteLevel(_tama_data.qte_level + 10);
          needs_save = true;
        } else if (qte.GetSuccess() >= QTE_TOTAL_ROUNDS - 2) {
          SetQteLevel(_tama_data.qte_level + 3);
          needs_save = true;
        }
        UpdateFrameBuffer();
      }
      break;
    case TAMA_APP_STATE::TRAINING_END:
      needs_render = true;
      if (_frame_count > 8) {
        _state = TAMA_APP_STATE::IDLE;
        UpdateFrameBuffer();
      }
      break;
    case TAMA_APP_STATE::PET_HEALING:
      needs_render = true;
      static_assert(TAMA_GET_ANIMATION_DATA(DOG_FED_HEALING).frame_count ==
                    TAMA_GET_ANIMATION_DATA(CAT_FED_HEALING).frame_count);
      if (_frame_count >
          TAMA_GET_ANIMATION_DATA(DOG_FED_HEALING).frame_count * 3) {
        _state = TAMA_APP_STATE::IDLE;
        UpdateFrameBuffer();
      }
      break;
    default:
      break;
  }

  if (needs_save && (_state != _tama_data.state)) {
    if (static_cast<uint8_t>(_state) &
        static_cast<uint8_t>(TAMA_APP_STATE::SAVE_STATE)) {
      _tama_data.state = _state;
    }
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
  switch (_state) {
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
          ConcateAnimtaions(2, pet, &TAMA_GET_ANIMATION_DATA(NEED_HEAL));
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
      StackOnFrame(&TAMA_NUM_FONT[GetDisplayLevel() / 100], 5);
      StackOnFrame(&TAMA_NUM_FONT[(GetDisplayLevel() % 100) / 10], 9);
      StackOnFrame(&TAMA_NUM_FONT[GetDisplayLevel() % 10], 13);
      for (int i = 0;
           (i < _tama_data.secret_level && i < TAMA_MAX_SECRET_LEVEL); i++) {
        const display_buf_t indicator = 1 << (i / (TAMA_MAX_SECRET_LEVEL / 2));
        const tama_display_component_t indicator_component = {
            .data = &indicator, .length = 1};
        StackOnFrameBlinking(
            &indicator_component,
            (DISPLAY_WIDTH - 1 - (i % (TAMA_MAX_SECRET_LEVEL / 2))));
      }
      break;
    case TAMA_APP_STATE::FEED_CONFIRM:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(FEED_CONFIRM).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(FEED_CONFIRM), 4);
      StackOnFrame(&TAMA_COMPONENT_N_FONT, 0);
      StackOnFrame(&TAMA_COMPONENT_Y_FONT, 13);
      StackOnFrameBlinking(&TAMA_COMPONENT_SELECTION_CURSOR,
                           _is_selected ? 13 : 0);
      break;
    case TAMA_APP_STATE::FEED_ANIME:
      TAMA_PREPARE_FB(_fb, TAMA_GET_ANIMATION_DATA(FEEDING).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(FEEDING), 4);
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
    case TAMA_APP_STATE::TRAINING_CONFIRM:
      TAMA_PREPARE_FB(_fb,
                      TAMA_GET_ANIMATION_DATA(TRAINING_CONFIRM).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(TRAINING_CONFIRM), 4);
      StackOnFrame(&TAMA_COMPONENT_N_FONT, 0);
      StackOnFrame(&TAMA_COMPONENT_Y_FONT, 13);
      StackOnFrameBlinking(&TAMA_COMPONENT_SELECTION_CURSOR,
                           _is_selected ? 13 : 0);
      break;
    case TAMA_APP_STATE::TRAINING: {
      const tama_ani_t* me = nullptr;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        me = &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        me = &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT);
      } else {
        my_assert(false);
      }
      TAMA_PREPARE_FB(_fb, me->frame_count);
      TAMA_COPY_FB(_fb, *me, 0);
      StackOnFrame(&TAMA_COMPONENT_TRAINING_FACILITY, 9);
      break;
    }
    case TAMA_APP_STATE::TRAINING_END: {
      const tama_ani_t* me = _tama_data.type == TAMA_TYPE::DOG
                                 ? &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG)
                                 : &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT);
      TAMA_PREPARE_FB(_fb, me->frame_count);
      TAMA_COPY_FB(_fb, *me, 4);
      if (qte.GetSuccess() == QTE_TOTAL_ROUNDS) {
        StackOnFrameBlinking(&TAMA_COMPONENT_TRAINING_LV_UP, 2);
      }
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
  if (((button & BUTTON_VALUE_MASK) == BUTTON_BACK) ||
      ((button & BUTTON_VALUE_MASK) == BUTTON_LONG_BACK)) {
    badge_controller.BackToMenu(this);
    return;
  }
  switch (my_packet.state) {
    case TAMA_XBOARD_STATE::XBOARD_INVITE:
      switch (button & BUTTON_VALUE_MASK) {
        case BUTTON_OK: {
          if (xboard_battle_invite ==
              TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N) {
            my_packet.state = TAMA_XBOARD_STATE::XBOARD_LEAVE;
            display_set_mode_scroll_text("Your pet fled...");
          } else {
            my_assert(xboard_battle_invite ==
                      TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_Y);
            my_packet.state = TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER;
            my_packet.type = _tama_data.type;
            display_set_mode_scroll_text("Waiting for enemy...");
          }
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
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE:
      qte.OnButton(button);
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER:
    case TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE:
    default:
      break;
  }
}
void TamaApp::XbUpdateFrameBuffer() {
  // TODO: Handle all XBoard frame here
  _frame_count = 0;
  switch (my_packet.state) {
    case TAMA_XBOARD_STATE::XBOARD_INVITE:
      TAMA_PREPARE_FB(_fb,
                      TAMA_GET_ANIMATION_DATA(XB_BATTLE_INVITE).frame_count);
      TAMA_COPY_FB(_fb, TAMA_GET_ANIMATION_DATA(XB_BATTLE_INVITE), 4);
      StackOnFrame(&TAMA_COMPONENT_N_FONT, 0);
      StackOnFrame(&TAMA_COMPONENT_Y_FONT, 13);
      StackOnFrameBlinking(
          &TAMA_COMPONENT_SELECTION_CURSOR,
          xboard_battle_invite == TAMA_XBOARD_BATTLE_INVITE::XBOARD_BATTLE_N
              ? 0
              : 13);
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER: {
      if (enemy_packet.state < TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER) {
        return;
      }
      const tama_ani_t *me = nullptr, *enemy = nullptr;
      if (_tama_data.type == TAMA_TYPE::DOG) {
        me = &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG);
      } else if (_tama_data.type == TAMA_TYPE::CAT) {
        me = &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT);
      } else {
        my_assert(false);
      }
      if (enemy_packet.type == TAMA_TYPE::DOG) {
        enemy = &TAMA_GET_ANIMATION_DATA(XB_ENEMY_DOG);
      } else if (enemy_packet.type == TAMA_TYPE::CAT) {
        enemy = &TAMA_GET_ANIMATION_DATA(XB_ENEMY_CAT);
      } else {
        my_assert(false);
      }
      ConcateAnimtaions(2, me, enemy);
      break;
    }
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_RESULT: {
      const tama_ani_t *me = nullptr, *enemy = nullptr;
      if (_xb_qte_me_winning) {
        me = _tama_data.type == TAMA_TYPE::DOG
                 ? &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG)
                 : &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT);
      } else {
        me = _tama_data.type == TAMA_TYPE::DOG
                 ? &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG_HURT)
                 : &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT_HURT);
      }
      if (_xb_qte_enemy_winning) {
        enemy = _tama_data.type == TAMA_TYPE::DOG
                    ? &TAMA_GET_ANIMATION_DATA(XB_ENEMY_DOG)
                    : &TAMA_GET_ANIMATION_DATA(XB_ENEMY_CAT);
      } else {
        enemy = _tama_data.type == TAMA_TYPE::DOG
                    ? &TAMA_GET_ANIMATION_DATA(XB_ENEMY_DOG_HURT)
                    : &TAMA_GET_ANIMATION_DATA(XB_ENEMY_CAT_HURT);
      }
      ConcateAnimtaions(2, me, enemy);
      break;
    }
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_END: {
      const tama_ani_t* me = _tama_data.type == TAMA_TYPE::DOG
                                 ? &TAMA_GET_ANIMATION_DATA(XB_PLAYER_DOG)
                                 : &TAMA_GET_ANIMATION_DATA(XB_PLAYER_CAT);
      TAMA_PREPARE_FB(_fb, me->frame_count);
      TAMA_COPY_FB(_fb, *me, 4);
      if (_xb_qte_me_winning) {
        StackOnFrameShifing(&TAMA_COMPONENT_QTE_WINNING_EFFECT, 0);
      } else {
        StackOnFrameBlinking(&TAMA_COMPONENT_QTE_LOSING_EFFECT, 2);
      }
      break;
    }
    default:
      break;
  }
}

void TamaApp::OnXBoardRecv(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  if ((TAMA_XBOARD_STATE)packet->data[0] == enemy_packet.state) return;
  // Packet has been updated, we now knows that enemy state has changed
  memcpy(&enemy_packet, packet->data, sizeof(enemy_packet));
  switch (enemy_packet.state) {
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER:
      my_assert(enemy_packet.type == TAMA_TYPE::DOG ||
                enemy_packet.type == TAMA_TYPE::CAT);
      UpdateFrameBuffer();
      break;
    case TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE:
      my_assert(enemy_packet.result.nonce);
      UpdateFrameBuffer();
      break;
    case TAMA_XBOARD_STATE::XBOARD_LEAVE:
      if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_LEAVE) {
        return;
      }
      display_set_mode_scroll_text("Enemy fled...");
      my_packet.state = TAMA_XBOARD_STATE::XBOARD_LEAVE;
      return;  // Exit immediately
    case TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE:
      if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE) {
        return;
      }
      display_set_mode_scroll_text("Enemy unavailable");
      my_packet.state = TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE;
      break;
    default:
      break;
  }
}

void TamaApp::XbRoutine(void* unused) {
  bool needs_save = false;
  // Keep sending packet to prevent packet lost
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&my_packet),
                                sizeof(my_packet), TAMA_RECV_ID);
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE ||
      my_packet.state == TAMA_XBOARD_STATE::XBOARD_LEAVE ||
      enemy_packet.state == TAMA_XBOARD_STATE::XBOARD_UNAVAILABLE) {
    // We can not battle now. Do nothing to let the display scroll
    return;
  }

  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
      (enemy_packet.state < TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER)) {
    // Let waiting scrolls
    return;
  }
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
      enemy_packet.state >= TAMA_XBOARD_STATE::XBOARD_BATTLE_ENCOUNTER &&
      _frame_count >= 8) {
    my_packet.state = TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE;
    qte.Entry();
    return;
  }
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_QTE) {
    if (qte.IsDone()) {
      display_set_mode_scroll_text("Waiting for enemy...");
      _my_nounce = (g_fast_random_pool.GetRandom() % (UINT16_MAX - 1)) + 1;

      my_packet.state = TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE;
      my_packet.result.score = qte.GetScore(GetCombatLevel()),
      my_packet.result.nonce = _my_nounce;
      g_game_controller.SetBufferToUsername(my_packet.result.user);

      UpdateFrameBuffer();
    }
    return;
  }
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
    if (enemy_packet.state < TAMA_XBOARD_STATE::XBOARD_BATTLE_SENT_SCORE) {
      return;
    }
    my_packet.state = TAMA_XBOARD_STATE::XBOARD_BATTLE_RESULT;
    // We need to know enemy score to update our frames
    my_assert(enemy_packet.result.nonce);
    // Send result with TwoBadgeActivity
    hitcon::game::TwoBadgeActivity activity = {
        .gameType = hitcon::game::EventType::kTama,
        .myScore = qte.GetScore(GetCombatLevel()),
        .otherScore = enemy_packet.result.score,
        .nonce = _my_nounce + enemy_packet.result.nonce,
    };
    memcpy(activity.otherUser, enemy_packet.result.user,
           sizeof(enemy_packet.result.user));
    g_game_controller.SendTwoBadgeActivity(activity);
    if (activity.myScore > activity.otherScore)
      _xb_qte_me_winning = true;
    else
      _xb_qte_me_winning = false;
    if (activity.otherScore > activity.myScore)
      _xb_qte_enemy_winning = true;
    else
      _xb_qte_enemy_winning = false;

    if (activity.otherScore == CRITICAL_HIT_SCORE)
      _tama_data.hp = 0;
    else if (_xb_qte_me_winning)
      SetQteLevel(_tama_data.qte_level + 3);
    else
      _tama_data.hp = _tama_data.hp ? _tama_data.hp - 1 : 0;
    needs_save = true;
    UpdateFrameBuffer();
  }
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_RESULT) {
    if (_frame_count >= 8) {
      my_packet.state = TAMA_XBOARD_STATE::XBOARD_BATTLE_END;
      UpdateFrameBuffer();
    }
  }
  if (my_packet.state == TAMA_XBOARD_STATE::XBOARD_BATTLE_END) {
    if (_frame_count >= 8) {
      badge_controller.BackToMenu(this);
    }
  }

  if (needs_save) g_nv_storage.MarkDirty();

  Render();
}

void TamaApp::TamaHeal() {
  _tama_data.hp = 3;
  SetHunger(4);
  _state = TAMA_APP_STATE::PET_HEALING;
  UpdateFrameBuffer();
}

void TamaApp::StackOnFrame(const tama_display_component_t* component,
                           int offset) {
  for (int i = 0; i < _fb.fb_size; i++) {
    for (int j = offset; (j < component->length + offset && j < DISPLAY_WIDTH);
         j++) {
      _fb.fb[i][j] |= (component->data)[j - offset];
    }
  }
}

void TamaApp::StackOnFrameBlinking(const tama_display_component_t* component,
                                   int offset) {
  if (_fb.fb_size % 2) {
    // Double the frames for blinking effects
    for (int i = 0; i < _fb.fb_size; i++) {
      memcpy(_fb.fb[_fb.fb_size + i], _fb.fb[i], sizeof(_fb.fb[i]));
    }
    _fb.fb_size *= 2;
  }
  for (int i = 0; i < _fb.fb_size; i += 2) {
    for (int j = offset; (j < component->length + offset && j < DISPLAY_WIDTH);
         j++) {
      _fb.fb[i][j] |= (component->data)[j - offset];
    }
  }
}

void TamaApp::StackOnFrameShifing(const tama_display_component_t* component,
                                  int offset) {
  if (_fb.fb_size % 2) {
    // Double the frames for shifting effects
    for (int i = 0; i < _fb.fb_size; i++) {
      memcpy(_fb.fb[_fb.fb_size + i], _fb.fb[i], sizeof(_fb.fb[i]));
    }
    _fb.fb_size *= 2;
  }
  for (int i = 0; i < _fb.fb_size; i += 2) {
    for (int j = offset; (j < component->length + offset && j < DISPLAY_WIDTH);
         j++) {
      _fb.fb[i][j] |= (component->data)[j - offset];
      if (j + 1 < DISPLAY_WIDTH) {
        _fb.fb[i + 1][j + 1] |= (component->data)[j - offset];
      }
    }
  }
}

void TamaApp::HungerRoutine(void* unused) {
  bool needs_save = false;
  if (_tama_data.hp == 0 || _tama_data.hunger == 0) return;

  unsigned int now = SysTimer::GetTime();
  _hunger_check_elapsed += now - _last_hunger_check;
  _last_hunger_check = now;
  if (_hunger_check_elapsed >= TAMA_HUNGER_DECREASE_INTERVAL) {
    SetHunger(_tama_data.hunger - 1);
    _hunger_check_elapsed = 0;
    if (_tama_data.hunger == 0) {
      _tama_data.hp = 0;
      UpdateFrameBuffer();
    }
    needs_save = true;
  }

  if (needs_save) g_nv_storage.MarkDirty();
}

static unsigned int IntSqrt(unsigned int x) {
  unsigned int res = 0;
  unsigned int bit = 1 << 30;
  while (bit > x) bit >>= 2;

  while (bit) {
    if (x >= res + bit) {
      x -= res + bit;
      res = (res >> 1) + bit;
    } else
      res >>= 1;
  }
  return res;
}

static inline unsigned int min(unsigned int a, unsigned int b) {
  return (a < b) ? a : b;
}

void TamaApp::LevelUpRoutine(void* unused) {
  bool needs_save = false;
  if (_tama_data.hp == 0 || _tama_data.hunger == 0) return;

  unsigned int step = g_imu_logic.GetStep();
  if (step > _last_level_check_steps)
    _level_up_progress += step - _last_level_check_steps;
  _last_level_check_steps = step;

  // Steps needed to level up = min(100, current_level ^ 1.5)
  if (_level_up_progress >=
      min(100, _tama_data.step_level * IntSqrt(_tama_data.step_level))) {
    SetStepLevel(_tama_data.step_level + 1);
    needs_save = true;
  }

  if (needs_save) g_nv_storage.MarkDirty();
}

void TamaApp::SetHunger(uint8_t hunger) {
  if (hunger > 4) hunger = 4;
  _tama_data.hunger = hunger;
  _hunger_check_elapsed = 0;
  _last_hunger_check = SysTimer::GetTime();
}

void TamaApp::SetQteLevel(uint16_t level) {
  if (level > 499) level = 499;
  _tama_data.qte_level = level;
}

void TamaApp::SetStepLevel(uint16_t level) {
  if (level > 499) level = 499;
  _tama_data.step_level = level;
  _level_up_progress = 0;
}

uint16_t TamaApp::GetDisplayLevel() const {
  return _tama_data.step_level + _tama_data.qte_level;
}

uint16_t TamaApp::GetCombatLevel() const {
  return _tama_data.step_level + _tama_data.qte_level + _tama_data.secret_level;
}

void TamaApp::HatchingRoutine(void* unused) {
  if (_state != TAMA_APP_STATE::EGG_1 && _state != TAMA_APP_STATE::EGG_2 &&
      _state != TAMA_APP_STATE::EGG_3 && _state != TAMA_APP_STATE::EGG_4) {
    return;
  }
  unsigned int step = g_imu_logic.GetStep();
  constexpr int hatching_delta = TAMA_HATCHING_STEPS / 4;
  if (step > _previous_hatching_step) {
    _total_hatchin_steps += step - _previous_hatching_step;
    if (_total_hatchin_steps >= TAMA_HATCHING_STEPS) {
      _total_hatchin_steps = TAMA_HATCHING_STEPS;
      _state = TAMA_APP_STATE::HATCHING;
      return;
    }
    switch (_total_hatchin_steps / hatching_delta) {
      case 0:
        _state = TAMA_APP_STATE::EGG_1;
        break;
      case 1:
        _state = TAMA_APP_STATE::EGG_2;
        break;
      case 2:
        _state = TAMA_APP_STATE::EGG_3;
        break;
      case 3:
        _state = TAMA_APP_STATE::EGG_4;
        break;
      default:
        my_assert(false);
        break;
    }
    if (_state != _tama_data.state) {
      UpdateFrameBuffer();
      _tama_data.state = _state;
      g_nv_storage.MarkDirty();
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

void TamaApp::SponsorRegister(uint8_t sponsor_id) {
  unsigned int mask = 1 << sponsor_id;
  if (_tama_data.sponsor_register & mask) {
    return;
  }
  _tama_data.secret_level++;
  _tama_data.sponsor_register |= mask;
  g_nv_storage.MarkDirty();
}

void TamaQte::Routine() {
  if (state == kInGame) {
    if (game.IsDone()) {
      if (game.IsSuccess()) ++success;
      if (++currentRound == QTE_TOTAL_ROUNDS) {
        SaveScore();
        Exit();
      } else {
        state = TamaQteState::kBetweenGame;
        nextGameStart = SysTimer::GetTime() + PAUSE_BETWEEN_QTE_GAMES;
      }
    }
    game.Update();
  } else if (state == kBetweenGame) {
    if (SysTimer::GetTime() >= nextGameStart) {
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
  success = 0;
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

void TamaQte::SaveScore() {
  if (success == QTE_TOTAL_ROUNDS)
    score = CRITICAL_HIT_SCORE;
  else if (success == QTE_TOTAL_ROUNDS - 1 &&
           g_fast_random_pool.GetRandom() & 1)
    score = CRITICAL_HIT_SCORE;
  else
    score = success * success;
}

uint8_t TamaQte::GetSuccess() { return success; }

uint16_t TamaQte::GetScore(uint16_t level) {
  if (score == CRITICAL_HIT_SCORE)
    return CRITICAL_HIT_SCORE;
  else
    return score * level;
}

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

bool TamaQteGame::IsSuccess() { return success; }

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
