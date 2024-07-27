#include "HardwareTestApp.h"

#include <App/EditNameApp.h>
#include <Logic/Display/display.h>
#include <Service/DisplayService.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/XBoardService.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::ir;

namespace hitcon {

constexpr uint8_t kIrBuffer1[] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
constexpr size_t kIrBuffer1Size = sizeof(kIrBuffer1) / sizeof(kIrBuffer1[0]);

constexpr uint8_t kIrBuffer2[] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
                                  0xFF, 0xFF, 0xFF, 0x00, 0x00};
constexpr size_t kIrBuffer2Size = sizeof(kIrBuffer2) / sizeof(kIrBuffer2[0]);

HardwareTestApp hardware_test_app;
HardwareTestApp::HardwareTestApp()
    : task(30, (task_callback_t)&HardwareTestApp::Routine, (void*)this,
           PERIOD / 5) {}

unsigned char _xboard_data[2] = {'R', 'A'};
void HardwareTestApp::CheckXBoard(void* arg1) {
  uint8_t b = static_cast<uint8_t>(reinterpret_cast<size_t>(arg1));

  if (b == _xboard_data[0] && _count == 0) {
    _count = 1;
  } else if (b == _xboard_data[1] && _count == 1) {
    _count = 2;
    InitIRTest();
    next_state = TS_IR;
  }
}

void HardwareTestApp::Init() {
  scheduler.Queue(&task, nullptr);
  g_xboard_service.SetOnByteRx((callback_t)&HardwareTestApp::CheckXBoard, this);
  irService.SetOnBufferReceived((callback_t)&HardwareTestApp::OnIrRx, this);
}

void HardwareTestApp::InitIRTest() {
  ir_found_3ff = ir_found_7ff = false;
  ir_ff_count = ir_nonff_count = 0;
  ir_state = 0;
}

void HardwareTestApp::OnIrRx(void* arg1) {
  uint8_t* irbuf = reinterpret_cast<uint8_t*>(arg1);
  for (int i = 0; i < IR_SERVICE_RX_ON_BUFFER_SIZE; i++) {
    if (irbuf[i] == 0x00) {
      CheckIrCount(ir_ff_count, ir_nonff_count);
      ir_ff_count = 0;
      ir_nonff_count = 0;
    } else if (irbuf[i] == 0xff) {
      ir_ff_count++;
    } else {
      ir_nonff_count++;
    }
  }
}

void HardwareTestApp::CheckIrCount(uint32_t ff_count, uint32_t ir_nonff_count) {
  if (ir_nonff_count >= 3) {
    return;
  }

  if (ff_count < 10) {
    return;
  }

  if (ff_count >= (3 * 4 - 2) && ff_count <= (3 * 4 + 1)) {
    ir_found_3ff = true;
  }

  if (ff_count >= (7 * 4 - 2) && ff_count <= (7 * 4 + 1)) {
    ir_found_7ff = true;
  }
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
        g_xboard_service.QueueDataForTx(_xboard_data, 2);
      }
      break;
    case TS_IR:
      if (button == BUTTON_OK) {
        _count = 0;
      }
      break;
  }
}

void HardwareTestApp::Routine(void* unused) {
  if (current_state != next_state) {
    current_state = next_state;
    char temp[5];
    temp[0] = current_state / 10 + '0';
    temp[1] = current_state % 10 + '0';
    temp[2] = 0;
    display_set_mode_scroll_text(temp);
  }
  // TEST IR
  if (current_state == TS_IR) {
    if (ir_state == 0 && irService.CanSendBufferNow()) {
      irService.SendBuffer(kIrBuffer1, kIrBuffer1Size, true);
      ir_state++;
    } else if (ir_state == 1 && irService.CanSendBufferNow()) {
      irService.SendBuffer(kIrBuffer2, kIrBuffer2Size, true);
      ir_state++;
    } else if (ir_state == 2) {
      ir_state = 0;
    }

    if (ir_found_3ff && ir_found_7ff) {
      next_state = TS_PASS;
    }
  }

  // TEST DISPLAY
  if (current_state < TS_BTN_BRIGHTNESS) {
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
    // clang-format on

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
        display_set_mode_fixed(buf_fixed2);
        if (HAL_GetTick() - start_tick > PERIOD) {
          next_state = TS_DISPLAY_BRIGHTNESS;
          start_tick = HAL_GetTick();
        }
        break;
      case TS_DISPLAY_BRIGHTNESS:
        if (_count > 10) next_state = TS_BTN_BRIGHTNESS;
        if (HAL_GetTick() - start_tick > PERIOD / 5) {
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
