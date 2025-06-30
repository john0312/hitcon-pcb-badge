#include "HardwareTestApp.h"

#include <App/EditNameApp.h>
#include <Logic/Display/display.h>
#include <Logic/ImuLogic.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Scheduler.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;
using namespace hitcon::ir;

namespace hitcon {
HardwareTestApp hardware_test_app;
HardwareTestApp::HardwareTestApp()
    : task(30, (task_callback_t)&HardwareTestApp::Routine, (void*)this,
           PERIOD / 5) {}

constexpr uint8_t _xboard_data[] = {'T', 'U', 'Z', 'K', 'I'};
constexpr uint8_t _xboard_data_len = sizeof(_xboard_data) / sizeof(uint8_t);
void HardwareTestApp::CheckXBoard(void* arg1) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg1);
  bool flag = true;
  if (packet->len != _xboard_data_len) flag = false;
  for (uint8_t i = 0; i < _xboard_data_len; i++) {
    if (_xboard_data[i] != packet->data[i]) {
      flag = false;
      break;
    }
  }
  if (flag) next_state = TS_IR;
}

void HardwareTestApp::Init() {
  scheduler.Queue(&task, nullptr);
  g_xboard_logic.SetOnPacketArrive((callback_t)&HardwareTestApp::CheckXBoard,
                                   this, TEST_APP_RECV_ID);
}

void HardwareTestApp::CheckIr(void* arg1) {
  ShowPacket* packet = reinterpret_cast<ShowPacket*>(arg1);

  size_t i;
  for (i = 0; i < _ir_data_len; i++) {
    if (packet->message[i] != _ir_data.opaq.show.message[i]) break;
  }
  if (i == _ir_data_len) {
#ifdef V1_1C
    next_state = TS_PASS;
#else
    next_state = TS_GYRO;
#endif
  }
}

void HardwareTestApp::CheckImu(void* arg) {
  bool pass = static_cast<bool>(arg);
  if (!pass) {
    next_state = TS_FAIL;
    return;
  }
  if (current_state == TS_GYRO) {
    g_imu_logic.AccSelfTest((callback_t)&HardwareTestApp::CheckImu, this);
    next_state = TS_ACC;
  } else if (current_state == TS_ACC)
    next_state = TS_PASS;
}

void HardwareTestApp::OnEntry() {
  next_state = TS_DISPLAY_SET_ALL;
  start_tick = HAL_GetTick();
  _count = 0;
  scheduler.EnablePeriodic(&task);
  irController.SetDisableBroadcast();
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
        g_xboard_logic.QueueDataForTx(const_cast<uint8_t*>(_xboard_data),
                                      _xboard_data_len, TEST_APP_RECV_ID);
      }
      break;
    case TS_IR:
      if (button == BUTTON_OK) {
        _count = 0;
        for (uint8_t i = 0; i < IR_TEST_LEN; i++) {
          _ir_data.opaq.show.message[i] = g_fast_random_pool.GetRandom() % 256;
        }

        _ir_data.ttl = 0;
        _ir_data.type = packet_type::kTest;
        _ir_data_len = IR_DATA_HEADER_SIZE + IR_TEST_LEN;
        irLogic.SendPacket(reinterpret_cast<uint8_t*>(&_ir_data), _ir_data_len);
      }
      break;
    case TS_GYRO:
      if (button == BUTTON_OK) {
        HAL_Delay(500);
        g_imu_logic.GyroSelfTest((callback_t)&HardwareTestApp::CheckImu, this);
      }
      break;
  }
}
// clang-format off
constexpr uint8_t buf_fixed[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

constexpr uint8_t buf_fixed2[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
};

constexpr uint8_t buf_fixed3[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
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
