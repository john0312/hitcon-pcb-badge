#include <Hitcon.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/ImuLogic.h>
#include <Logic/lsm6ds3tr-c_reg.h>
#include <Service/ImuService.h>
#include <Service/Sched/SysTimer.h>

#include <cstdio>
#include <cstring>

using namespace hitcon;

namespace hitcon {

ImuLogic g_imu_logic;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
ImuLogic::ImuLogic()
    : _routine_task(419, (task_callback_t)&ImuLogic::Routine, (void*)this,
                    ROUTINE_INTERVAL),
      _proximity_task(420, (task_callback_t)&ImuLogic::ProximityRoutine,
                      (void*)this, PROXIMITY_INTERVAL),
      _state(RoutineState::INIT), _init_state(InitState::CHECK_ID), _step(0) {}
#pragma GCC diagnostic pop

void ImuLogic::Init() {
#ifdef V1_1
  _state = RoutineState::IDLE;
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_imu_service.SetRxCallback((callback_t)&ImuLogic::OnRxDone, this);
  g_imu_service.SetTxCallback((callback_t)&ImuLogic::OnTxDone, this);
#pragma GCC diagnostic pop
  _start_time = SysTimer::GetTime();
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  scheduler.Queue(&_proximity_task, nullptr);
  scheduler.EnablePeriodic(&_proximity_task);
}

void ImuLogic::Reset() {
  _state = RoutineState::INIT;
  _init_state = InitState::CHECK_ID;
  _start_time = SysTimer::GetTime();
#ifdef DEBUG
  _reset_cnt_without_success++;
  if (kEnableAssertionSTForUnresponsiveI2C) {
    if (_reset_cnt_without_success >= 8) {
      display_set_mode_text("ST");
      if (_reset_cnt_without_success >= 11) my_assert(false);
    }
  }
#endif
}

void ImuLogic::GyroSelfTest(callback_t cb, void* cb_arg1) {
  _count = 0;
  memset(_avg_values, 0, sizeof(_avg_values));
  _self_test_state = SelfTestState::SW_RESET;
  _gyro_st_cb = cb;
  _gyro_st_cb_arg1 = cb_arg1;
  _state = RoutineState::ST_GYRO;
}

void ImuLogic::AccSelfTest(callback_t cb, void* cb_arg1) {
  _count = 0;
  memset(_avg_values, 0, sizeof(_avg_values));
  _self_test_state = SelfTestState::SW_RESET;
  _acc_st_cb = cb;
  _acc_st_cb_arg1 = cb_arg1;
  _state = RoutineState::ST_ACC;
}

void ImuLogic::Routine(void* arg1) {
#if defined(DUMMY_STEP) || defined(V1_1)
  // add one step every 5 seconds
  static uint8_t dummy_count = 0;
  if (dummy_count >= 5000 / ROUTINE_INTERVAL) {
    dummy_count = 0;
    Increment();
  } else {
    dummy_count++;
  }
#endif
  if (_state == RoutineState::IDLE) return;

  static RoutineState last_state = RoutineState::INIT;
  static uint16_t count = 0;
  if (last_state != _state) {
    last_state = _state;
    count = 0;
  } else
    count++;

  // if state stuck for 5s reset I2C
  if (count >= 50000 / ROUTINE_INTERVAL) {
    count = 0;
    g_imu_service.ResetI2C();
    g_imu_logic.Reset();
    return;
  }

  if (g_imu_service.IsBusy()) return;
  if (_state == RoutineState::WAIT_INIT_DELAY &&
      SysTimer::GetTime() - _start_time >= 50) {
    _state = RoutineState::INIT;
  } else if (_state == RoutineState::INIT) {
    switch (_init_state) {
      case InitState::CHECK_ID:
        g_imu_service.QueueReadReg(LSM6DS3TR_C_WHO_AM_I, _buf);
        _init_state = InitState::WAIT_ID;
        break;
      case InitState::SW_RESET: {
        lsm6ds3tr_c_ctrl3_c_t ctrl3_c = {0};
        ctrl3_c.sw_reset = PROPERTY_ENABLE;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL3_C, ctrl3_c);
        g_imu_service.QueueReadReg(LSM6DS3TR_C_CTRL3_C, _buf);
        _init_state = InitState::WAIT_SW_RESET;
        break;
      }
      case InitState::RESET_STEP_COUNT: {
        _last_step_reg = 0;
        lsm6ds3tr_c_ctrl10_c_t ctrl10_c = {0};
        ctrl10_c.pedo_rst_step = PROPERTY_ENABLE;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL10_C, ctrl10_c);
        uint8_t temp = 0xAA;  // reset timestamp
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_TIMESTAMP2_REG, temp);
        _init_state = InitState::WAIT_RESET_COUNT;
        break;
      }
      case InitState::CONFIGURE: {
        lsm6ds3tr_c_ctrl1_xl_t ctrl1_xl = {0};
        ctrl1_xl.odr_xl = LSM6DS3TR_C_XL_ODR_416Hz;
        ctrl1_xl.fs_xl = LSM6DS3TR_C_2g;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL1_XL, ctrl1_xl);
        lsm6ds3tr_c_ctrl10_c_t ctrl10_c = {0};
        ctrl10_c.func_en = PROPERTY_ENABLE;
        ctrl10_c.pedo_en = PROPERTY_ENABLE;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL10_C, ctrl10_c);
        _init_state = InitState::WAIT_CONFIGURE;
        break;
      }
    }
  } else if (_state == RoutineState::ST_GYRO) {
    switch (_self_test_state) {
      case SelfTestState::SW_RESET: {
        lsm6ds3tr_c_ctrl3_c_t ctrl3_c = {0};
        ctrl3_c.sw_reset = PROPERTY_ENABLE;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL3_C, ctrl3_c);
        g_imu_service.QueueReadReg(LSM6DS3TR_C_CTRL3_C, _buf);
        _self_test_state = SelfTestState::WAIT_SW_RESET;
        break;
      }
      case SelfTestState::CONFIGURE: {
        lsm6ds3tr_c_ctrl2_g_t ctrl2_g = {0};
        ctrl2_g.odr_g = LSM6DS3TR_C_GY_ODR_208Hz;
        ctrl2_g.fs_g = LSM6DS3TR_C_2000dps;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL2_G, ctrl2_g);
        lsm6ds3tr_c_ctrl3_c_t ctrl3_c = {0};
        ctrl3_c.if_inc = PROPERTY_ENABLE;  // default value
        ctrl3_c.bdu = PROPERTY_ENABLE;     // block data update
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL3_C, ctrl3_c);
        _self_test_state = SelfTestState::WAIT_CONFIGURE;
        break;
      }
      case SelfTestState::WAIT_A: {
        if (SysTimer::GetTime() - _start_time >= WAIT_TIME_GYRO_A)
          _self_test_state = SelfTestState::CHECK_DA;
        else
          break;
      }
      case SelfTestState::WAIT_B: {
        if (SysTimer::GetTime() - _start_time >= WAIT_TIME_GYRO_B)
          _self_test_state = SelfTestState::CHECK_DA;
        else
          break;
      }
      case SelfTestState::CHECK_DA: {
        g_imu_service.QueueReadReg(LSM6DS3TR_C_STATUS_REG, _buf);
        _self_test_state = SelfTestState::WAIT_DA;
        break;
      }
      case SelfTestState::READ_OUT: {
        for (uint8_t i = 0; i < 6; i++)  // read gyro three axis and 16 bits
          g_imu_service.QueueReadReg(LSM6DS3TR_C_OUTX_L_G + i, _buf + i);
        _self_test_state = SelfTestState::WAIT_READ_OUT;
        break;
      }
      case SelfTestState::ENABLE_SELF_TEST: {
        lsm6ds3tr_c_ctrl5_c_t ctrl5_c = {0};
        ctrl5_c.st_g = PROPERTY_ENABLE;  // enable self test
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL5_C, ctrl5_c);
        _self_test_state = SelfTestState::WAIT_B;
        _start_time = SysTimer::GetTime();
        break;
      }
      case SelfTestState::VERIFY: {
        float_t test_val[3];
        for (uint8_t j = 0; j < 3; j++) {
          test_val[j] = fabsf((_avg_values[0][j] - _avg_values[1][j]));
        }
        // check self test limit
        bool pass = true;
        for (uint8_t j = 0; j < 3; j++) {
          if ((MIN_ST_LIMIT_mdps > test_val[j]) ||
              (test_val[j] > MAX_ST_LIMIT_mdps)) {
            pass = false;
          }
        }

        // disable gyro sensor and self test
        uint8_t zero = 0;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL2_G, zero);
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL5_C, zero);
        _self_test_state = SelfTestState::DONE;
        _state = RoutineState::IDLE;
        if (_gyro_st_cb)
          _gyro_st_cb(_gyro_st_cb_arg1, reinterpret_cast<void*>(pass));
        break;
      }
    }
  } else if (_state == RoutineState::ST_ACC) {
    switch (_self_test_state) {
      case SelfTestState::SW_RESET: {
        lsm6ds3tr_c_ctrl3_c_t ctrl3_c = {0};
        ctrl3_c.sw_reset = PROPERTY_ENABLE;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL3_C, ctrl3_c);
        g_imu_service.QueueReadReg(LSM6DS3TR_C_CTRL3_C, _buf);
        _self_test_state = SelfTestState::WAIT_SW_RESET;
        break;
      }
      case SelfTestState::CONFIGURE: {
        lsm6ds3tr_c_ctrl1_xl_t ctrl1_xl = {0};
        ctrl1_xl.odr_xl = LSM6DS3TR_C_XL_ODR_52Hz;
        ctrl1_xl.fs_xl = LSM6DS3TR_C_4g;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL1_XL, ctrl1_xl);
        lsm6ds3tr_c_ctrl3_c_t ctrl3_c = {0};
        ctrl3_c.if_inc = PROPERTY_ENABLE;  // default value
        ctrl3_c.bdu = PROPERTY_ENABLE;     // block data update
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL3_C, ctrl3_c);
        _self_test_state = SelfTestState::WAIT_CONFIGURE;
        break;
      }
      case SelfTestState::WAIT_A: {
        if (SysTimer::GetTime() - _start_time >= WAIT_TIME_ACC_A)
          _self_test_state = SelfTestState::CHECK_DA;
        else
          break;
      }
      case SelfTestState::WAIT_B: {
        if (SysTimer::GetTime() - _start_time >= WAIT_TIME_ACC_B)
          _self_test_state = SelfTestState::CHECK_DA;
        else
          break;
      }
      case SelfTestState::CHECK_DA: {
        g_imu_service.QueueReadReg(LSM6DS3TR_C_STATUS_REG, _buf);
        _self_test_state = SelfTestState::WAIT_DA;
        break;
      }
      case SelfTestState::READ_OUT: {
        for (uint8_t i = 0; i < 6; i++)  // read gyro three axis and 16 bits
          g_imu_service.QueueReadReg(LSM6DS3TR_C_OUTX_L_XL + i, _buf + i);
        _self_test_state = SelfTestState::WAIT_READ_OUT;
        break;
      }
      case SelfTestState::ENABLE_SELF_TEST: {
        lsm6ds3tr_c_ctrl5_c_t ctrl5_c = {0};
        ctrl5_c.st_xl = PROPERTY_ENABLE;  // enable self test
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL5_C, ctrl5_c);
        _self_test_state = SelfTestState::WAIT_ENABLE_ST;
        break;
      }
      case SelfTestState::VERIFY: {
        float_t test_val[3];
        for (uint8_t i = 0; i < 3; i++) {
          test_val[i] = fabsf((_avg_values[0][i] - _avg_values[1][i]));
        }
        // check self test limit
        bool pass = true;
        for (uint8_t i = 0; i < 3; i++) {
          if ((MIN_ST_LIMIT_mg > test_val[i]) ||
              (test_val[i] > MAX_ST_LIMIT_mg)) {
            pass = false;
          }
        }

        // disable acc sensor and self test
        uint8_t zero = 0;
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL1_XL, zero);
        g_imu_service.QueueWriteReg(LSM6DS3TR_C_CTRL5_C, zero);
        _self_test_state = SelfTestState::DONE;
        _state = RoutineState::IDLE;
        if (_acc_st_cb)
          _acc_st_cb(_acc_st_cb_arg1, reinterpret_cast<void*>(pass));
        break;
      }
    }
  } else if (_state == RoutineState::GET_STEP) {
    g_imu_service.QueueReadReg(LSM6DS3TR_C_STEP_COUNTER_L, &_buf[0]);
    g_imu_service.QueueReadReg(LSM6DS3TR_C_STEP_COUNTER_H, &_buf[1]);
    _state = RoutineState::WAIT_STEP;
  }
}

void ImuLogic::OnRxDone(void* arg1) {
  switch (_init_state) {
    case InitState::WAIT_ID:
      my_assert(_buf[0] == LSM6DS3TR_C_ID);
      _init_state = InitState::SW_RESET;
      break;
    case InitState::WAIT_SW_RESET: {
      auto ctrl3_c = reinterpret_cast<lsm6ds3tr_c_ctrl3_c_t*>(_buf);
      if (ctrl3_c->sw_reset)  // check if sw reset is done
        g_imu_service.QueueReadReg(LSM6DS3TR_C_CTRL3_C, _buf);
      else
        _init_state = InitState::RESET_STEP_COUNT;
      break;
    }
    case InitState::WAIT_CONFIGURE:
      _state = RoutineState::IDLE;
      break;
  }

  switch (_self_test_state) {
    case SelfTestState::WAIT_SW_RESET: {
      auto ctrl3_c = reinterpret_cast<lsm6ds3tr_c_ctrl3_c_t*>(_buf);
      if (ctrl3_c->sw_reset)  // check if sw reset is done
        g_imu_service.QueueReadReg(LSM6DS3TR_C_CTRL3_C, _buf);
      else
        _self_test_state = SelfTestState::CONFIGURE;
      break;
    }
    case SelfTestState::WAIT_DA: {
      auto status = reinterpret_cast<lsm6ds3tr_c_status_reg_t*>(_buf);
      if ((status->xlda && _state == RoutineState::ST_ACC) ||
          (status->gda && _state == RoutineState::ST_GYRO))
        _self_test_state = SelfTestState::READ_OUT;
      else
        g_imu_service.QueueReadReg(LSM6DS3TR_C_STATUS_REG, _buf);
      break;
    }
    case SelfTestState::WAIT_READ_OUT: {
      int16_t data_raw[3] = {_buf[0] | (_buf[1] << 8), _buf[2] | (_buf[3] << 8),
                             _buf[4] | (_buf[5] << 8)};
      if (_count != 0 && _count != 6) {  // discard first data
        for (uint8_t i = 0; i < 3; i++) {
          if (_state == RoutineState::ST_GYRO)
            _avg_values[_count / 6][i] += 0.2f * ((float_t)data_raw[i] * 70.0f);
          else if (_state == RoutineState::ST_ACC)
            _avg_values[_count / 6][i] +=
                0.2f * ((float_t)data_raw[i] * 0.122f);
        }
      }
      if (_count == 5)
        _self_test_state = SelfTestState::ENABLE_SELF_TEST;
      else if (_count == 11)
        _self_test_state = SelfTestState::VERIFY;
      else
        _self_test_state = SelfTestState::CHECK_DA;
      _count++;
      break;
    }
  }

  if (_state == RoutineState::WAIT_STEP) {
#ifdef DEBUG
    _reset_cnt_without_success = 0;
#endif
    uint16_t val = _buf[0] | (_buf[1] << 8);
    uint16_t increment = 0;
    if (val < _last_step_reg) {  // handle STEP_COUNTER (16bit) overflow
      increment = (UINT16_MAX - _last_step_reg + 1) + val;
    } else {
      increment = (val - _last_step_reg);
    }
    // increment must be reasonable
    if (increment > SHAKING_THRESHOLD * 2) increment = SHAKING_THRESHOLD * 2;
    _step += increment;
    _last_step_reg = val;
    _is_shaking = (increment >= SHAKING_THRESHOLD);
    _state = RoutineState::GET_STEP;
  }
}

void ImuLogic::OnTxDone(void* arg1) {
  if (_init_state == InitState::WAIT_RESET_COUNT) {
    _init_state = InitState::CONFIGURE;
  } else if (_init_state == InitState::WAIT_CONFIGURE) {
    _state = RoutineState::GET_STEP;
    _init_state = InitState::DONE;
  } else if (_self_test_state == SelfTestState::WAIT_CONFIGURE) {
    _self_test_state = SelfTestState::WAIT_A;
    _start_time = SysTimer::GetTime();
  } else if (_self_test_state == SelfTestState::WAIT_ENABLE_ST) {
    _self_test_state = SelfTestState::WAIT_B;
    _start_time = SysTimer::GetTime();
  }
}

void ImuLogic::ProximityRoutine(void* arg1) {
  static uint32_t last_step = 0;
  hitcon::game::Proximity data;
  uint16_t temp = (GetStep() - last_step) / SCALE_FACTOR;
  if (temp > 255) temp = 255;
  data.power = temp;
  data.nonce = SysTimer::GetTime() & 0xFFFF;
  g_game_controller.SendProximity(data);
  last_step = GetStep();
}
}  // namespace hitcon
