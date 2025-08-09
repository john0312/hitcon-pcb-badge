#include <Logic/ImuLogic.h>
#include <Service/ImuService.h>
#include <Service/Sched/SysTimer.h>

#include "stm32f1xx_hal_i2c.h"

using namespace hitcon;

namespace {
void I2CCallbackWrapper(I2C_HandleTypeDef* hi2c) {
  if (hi2c == I2C_HANDLE) g_imu_service.I2CCallback();
}

void I2CErrorCallback(I2C_HandleTypeDef* hi2c) {
  static uint8_t i = 0;
  static unsigned tick = 0;
  if (hi2c == I2C_HANDLE) {
    // Only freeze when consecutive errors occurred
    if (SysTimer::GetTime() - tick < 1000)
      i++;
    else
      i = 0;
    tick = SysTimer::GetTime();
    if (i > 10) my_assert(false);

    g_imu_service.ResetI2C();
    g_imu_logic.Reset();
  }
}
}  // namespace

namespace hitcon {

ImuService g_imu_service;

ImuService::ImuService()
    : _routine_task(417, (task_callback_t)&ImuService::Routine, this, 100) {}

void ImuService::Init() {
  state = State::INIT;
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
}

void ImuService::ResetI2C() {
  HAL_I2C_DeInit(I2C_HANDLE);
  HAL_I2C_Init(I2C_HANDLE);
  state = State::INIT;
}

void ImuService::QueueReadReg(uint8_t addr, uint8_t* value) {
  if (_rx_queue.IsFull()) my_assert(false);

  ReadOp op = {addr, value};
  _rx_queue.PushFront(op);
  _is_rx_done = false;
}

void ImuService::QueueWriteReg(uint8_t addr, uint8_t value) {
  if (_tx_queue.IsFull()) my_assert(false);

  WriteOp op = {addr, value};
  _tx_queue.PushFront(op);
  _is_tx_done = false;
}

void ImuService::SetRxCallback(callback_t callback, void* callback_arg1) {
  _rx_cb = callback;
  _rx_cb_arg1 = callback_arg1;
}

void ImuService::SetTxCallback(callback_t callback, void* callback_arg1) {
  _tx_cb = callback;
  _tx_cb_arg1 = callback_arg1;
}

void ImuService::I2CCallback() {
  if (state == State::READING) {
    _rx_queue.PopBack();

    if (_rx_queue.IsEmpty() && _rx_cb != nullptr && !_is_rx_done) {
      _rx_cb(_rx_cb_arg1, nullptr);
      _is_rx_done = true;
    }
  } else if (state == State::WRITING) {
    _tx_queue.PopBack();

    if (_tx_queue.IsEmpty() && _tx_cb != nullptr && !_is_tx_done) {
      _tx_cb(_tx_cb_arg1, nullptr);
      _is_tx_done = true;
    }
  }

  state = State::IDLE;
}

void ImuService::Routine(void* arg) {
  static unsigned op_start_tick = 0;

  switch (state) {
    case State::INIT:
      _is_rx_done = true;
      _is_tx_done = true;
      _rx_queue.Clear();
      _tx_queue.Clear();

      HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MEM_RX_COMPLETE_CB_ID,
                               I2CCallbackWrapper);
      HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MEM_TX_COMPLETE_CB_ID,
                               I2CCallbackWrapper);
      HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MASTER_RX_COMPLETE_CB_ID,
                               I2CCallbackWrapper);
      HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MASTER_TX_COMPLETE_CB_ID,
                               I2CCallbackWrapper);
      HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_ERROR_CB_ID, I2CErrorCallback);
#ifdef V2_2
      HAL_GPIO_WritePin(IMU_PWR_GPIO_Port, IMU_PWR_Pin, GPIO_PIN_SET);
      op_start_tick = SysTimer::GetTime();
      state = State::WAIT_200_1;
#else
      state = State::IDLE;
#endif
      break;
#ifdef V2_2
    case State::WAIT_200_1:
      if (SysTimer::GetTime() - op_start_tick >= 200) {
        state = State::WAIT_200_2;
        op_start_tick = SysTimer::GetTime();
        HAL_GPIO_WritePin(IMU_PWR_GPIO_Port, IMU_PWR_Pin, GPIO_PIN_RESET);
      }
      break;
    case State::WAIT_200_2:
      if (SysTimer::GetTime() - op_start_tick >= 200) state = State::IDLE;
      break;
#endif
    case State::READING:
    case State::WRITING:
      // check if I2C interrupt stucked
      if (SysTimer::GetTime() - op_start_tick >= TIMEOUT)
        I2CErrorCallback(I2C_HANDLE);
      break;
    case State::IDLE: {
      HAL_StatusTypeDef status = HAL_OK;
      if (!_tx_queue.IsEmpty()) {
        WriteOp& op = _tx_queue.Back();
        state = State::WRITING;

        status = HAL_I2C_Mem_Write_IT(I2C_HANDLE, SLAVE_ADDR, op.addr,
                                      I2C_MEMADD_SIZE_8BIT, &op.value, 1);
        op_start_tick = SysTimer::GetTime();
      } else if (!_rx_queue.IsEmpty()) {
        ReadOp& op = _rx_queue.Back();
        state = State::READING;

        status = HAL_I2C_Mem_Read_IT(I2C_HANDLE, SLAVE_ADDR, op.addr,
                                     I2C_MEMADD_SIZE_8BIT, op.value_ptr, 1);
        op_start_tick = SysTimer::GetTime();
      }
      if (status != HAL_OK) {
        state = State::IDLE;
        my_assert(false);
      }
      break;
    }
  }
}

}  // namespace hitcon
