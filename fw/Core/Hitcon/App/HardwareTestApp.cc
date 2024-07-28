#include "HardwareTestApp.h"

#include <App/EditNameApp.h>
#include <Logic/Display/display.h>
#include <Logic/IrLogic.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/XBoardService.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::ir;

namespace hitcon {
HardwareTestApp hardware_test_app;
HardwareTestApp::HardwareTestApp()
    : task(30, (task_callback_t)&HardwareTestApp::Routine, (void*)this,
           PERIOD / 5) {}

constexpr uint8_t _xboard_data[] = {'R', 'A'};
constexpr uint8_t _xboard_data_len = sizeof(_xboard_data) / sizeof(uint8_t);
void HardwareTestApp::CheckXBoard(void* arg1) {
  uint8_t b = static_cast<uint8_t>(reinterpret_cast<size_t>(arg1));

  if (b == _xboard_data[0] && _count == 0) {
    _count = 1;
  } else if (b == _xboard_data[1] && _count == 1) {
    _count = 2;
    next_state = TS_IR;
  }
}

void HardwareTestApp::Init() {
  scheduler.Queue(&task, nullptr);
  g_xboard_service.SetOnByteRx((callback_t)&HardwareTestApp::CheckXBoard,
                               this);  // TODO: change to XBoardLogic
  irLogic.SetOnPacketReceived((callback_t)&HardwareTestApp::CheckIr, this);
}

constexpr uint8_t _ir_data[] = {'T', 'U', 'Z', 'K', 'I'};
constexpr uint8_t _ir_data_len = sizeof(_ir_data) / sizeof(uint8_t);
void HardwareTestApp::CheckIr(void* arg1) {
  IrPacket* packet = reinterpret_cast<IrPacket*>(arg1);
  size_t i;
  for (i = 0; i < _ir_data_len; i++) {
    if (packet->data_[i] != _ir_data[i]) break;
  }
  next_state = (i == _ir_data_len) ? TS_PASS : TS_FAIL;
}

void HardwareTestApp::OnEntry() {
  next_state = TS_DISPLAY_SET_ALL;
  start_tick = HAL_GetTick();
  _count = 0;
  scheduler.EnablePeriodic(&task);
}

void HardwareTestApp::OnExit() { scheduler.DisablePeriodic(&task); }

void HardwareTestApp::OnButton(button_t button) {
  switch (current_state) {
    case TS_BTN_BRIGHTNESS:
      if (button == BUTTON_BRIGHTNESS) {
        next_state = TS_BTN_BACK;
      }
      break;
    case TS_BTN_BACK:
      if (button == BUTTON_BACK) {
        next_state = TS_BTN_UP;
      }
      break;
    case TS_BTN_UP:
      if (button == BUTTON_UP) {
        next_state = TS_BTN_MODE;
      }
      break;
    case TS_BTN_MODE:
      if (button == BUTTON_MODE) {
        next_state = TS_BTN_LEFT;
      }
      break;
    case TS_BTN_LEFT:
      if (button == BUTTON_LEFT) {
        next_state = TS_BTN_OK;
      }
      break;
    case TS_BTN_OK:
      if (button == BUTTON_OK) {
        next_state = TS_BTN_RIGHT;
      }
      break;
    case TS_BTN_RIGHT:
      if (button == BUTTON_RIGHT) {
        next_state = TS_BTN_DOWN;
      }
      break;
    case TS_BTN_DOWN:
      if (button == BUTTON_DOWN) {
        next_state = TS_XBOARD;
      }
      break;
    case TS_XBOARD:
      if (button == BUTTON_OK) {
        _count = 0;
        g_xboard_service.QueueDataForTx(const_cast<uint8_t*>(_xboard_data),
                                        _xboard_data_len);
      }
      break;
    case TS_IR:
      if (button == BUTTON_OK) {
        _count = 0;
        irLogic.SendPacket(const_cast<uint8_t*>(_ir_data), _ir_data_len);
      }
      break;
  }
}
// clang-format off
uint8_t buf_fixed[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

uint8_t buf_fixed2[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
};

uint8_t buf_fixed3[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
};
// clang-format on

void HardwareTestApp::Routine(void* unused) {
  if (current_state != next_state) {
    current_state = next_state;
    char temp[5];
    temp[0] = current_state / 10 + '0';
    temp[1] = current_state % 10 + '0';
    temp[2] = 0;
    display_set_mode_text(temp);
  }

  // TEST DISPLAY
  if (current_state < TS_BTN_BRIGHTNESS) {
    switch (current_state) {
      case TS_DISPLAY_SET_ALL:
        display_set_mode_fixed(buf_fixed);
        if (HAL_GetTick() - start_tick > PERIOD) {
          next_state = TS_DISPLAY_RESET_ALL;
          start_tick = HAL_GetTick();
        }
        break;
      case TS_DISPLAY_RESET_ALL:
        display_set_mode_blank();
        if (HAL_GetTick() - start_tick > PERIOD) {
          next_state = TS_DISPLAY_CHECKER;
          start_tick = HAL_GetTick();
        }
        break;
      case TS_DISPLAY_CHECKER:
        display_set_mode_fixed(_count ? buf_fixed2 : buf_fixed3);
        if (HAL_GetTick() - start_tick > PERIOD) {
          if (_count == 0) {
            _count++;
          } else {
            next_state = TS_DISPLAY_BRIGHTNESS;
          }
          start_tick = HAL_GetTick();
        }
        break;
      case TS_DISPLAY_BRIGHTNESS:
        if (_count > 10) next_state = TS_BTN_BRIGHTNESS;
        if (HAL_GetTick() - start_tick > PERIOD / 10) {
          g_display_service.SetBrightness(_count);
          _count++;
          start_tick = HAL_GetTick();
        }
        break;
      default:
        break;
    }
  }
}

}  // namespace hitcon
