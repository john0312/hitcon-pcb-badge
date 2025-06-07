#include <Service/ImuService.h>

#include <cstdio>
#include <cstring>

#include "i2c.h"
#include "main.h"

using namespace hitcon;
using namespace hitcon::service::sched;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == IMU_INT1_Pin) {
  }
}

namespace hitcon {
ImuService g_imu_service;

ImuService::ImuService()
    : _init_task(418, (task_callback_t)&ImuService::LSM6DS3Init, (void *)this,
                 15),
      _routine_task(417, (task_callback_t)&ImuService::Routine, (void *)this,
                    200),
      _self_test_acc_task(416, (task_callback_t)&ImuService::AccSelfTest,
                          (void *)this, 100),
      _self_test_gyro_task(415, (task_callback_t)&ImuService::GyroSelfTest,
                           (void *)this, 150),
      rx_ready(true), tx_ready(true), _state(State::INIT) {}

void RegRxCallback(I2C_HandleTypeDef *hi2c) { g_imu_service.rx_ready = true; }

void RegTxCallback(I2C_HandleTypeDef *hi2c) { g_imu_service.tx_ready = true; }

void ImuService::Init() {
  _state = State::INIT;
  _dev_ctx.write_reg = platform_write;
  _dev_ctx.read_reg = platform_read;
  _dev_ctx.mdelay = platform_delay;
  _dev_ctx.handle = &hi2c1;
  HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MEM_RX_COMPLETE_CB_ID,
                           RegRxCallback);
  HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MEM_TX_COMPLETE_CB_ID,
                           RegTxCallback);
  HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MASTER_RX_COMPLETE_CB_ID,
                           RegRxCallback);
  HAL_I2C_RegisterCallback(&hi2c1, HAL_I2C_MASTER_TX_COMPLETE_CB_ID,
                           RegTxCallback);
  scheduler.Queue(&_init_task, nullptr);
  scheduler.Queue(&_routine_task, nullptr);
}

void ImuService::LSM6DS3Init(void *arg1) {
  int32_t status;
  status = lsm6ds3tr_c_device_id_get(&_dev_ctx, &_rx_buf);
  if (status != HAL_OK || _rx_buf != LSM6DS3TR_C_ID) my_assert(false);
  SwReset();

  lsm6ds3tr_c_xl_data_rate_set(&_dev_ctx, LSM6DS3TR_C_XL_ODR_26Hz);
  lsm6ds3tr_c_xl_full_scale_set(&_dev_ctx, LSM6DS3TR_C_2g);
  lsm6ds3tr_c_func_en_set(&_dev_ctx, PROPERTY_ENABLE);
  lsm6ds3tr_c_pedo_sens_set(&_dev_ctx, PROPERTY_ENABLE);

  _state = State::IDLE;
  scheduler.EnablePeriodic(&_routine_task);
}

void ImuService::QueueReadReg(uint8_t addr) {
  rx_ready = false;
  HAL_I2C_Mem_Read_IT(&I2C_HANDLE, LSM6DS3TR_C_I2C_ADD_L, addr, 1, &_rx_buf, 1);
}

void ImuService::Routine(void *arg1) {
  if (!rx_ready) return;
}

void ImuService::AccSelfTest(void *cb) {
  static callback_t callback = reinterpret_cast<callback_t>(cb);

  static int16_t data_raw[3];
  // avg_values[0]: OUT_NOST, avg_values[1]: OUT_ST
  static float_t avg_values[2][3];
  static uint8_t i;
  // TODO: check runtime within 1.5ms
  if (_state == State::IDLE) {
    _state = State::SELF_TEST_ACC;
    scheduler.DisablePeriodic(&_routine_task);
    SwReset();
    // initialize sensor, set FS=4G, ODR=52Hz, BDU=1
    lsm6ds3tr_c_xl_full_scale_set(&_dev_ctx, LSM6DS3TR_C_4g);
    lsm6ds3tr_c_xl_data_rate_set(&_dev_ctx, LSM6DS3TR_C_XL_ODR_52Hz);
    lsm6ds3tr_c_block_data_update_set(&_dev_ctx, PROPERTY_ENABLE);

    // wait 100ms for stable output
    _self_test_acc_task.SetWakeTime(100);
    scheduler.Queue(&_self_test_acc_task, nullptr);
    memset(avg_values, 0, 2 * 3 * sizeof(float_t));
    i = 0;
    return;
  }

  if (i < 10) {
    if (i == 0 || i == 5) {
      // check acc data ready bit, wait for first sample
      do {
        lsm6ds3tr_c_xl_flag_data_ready_get(&_dev_ctx, &_rx_buf);
      } while (!_rx_buf);

      // read and discard data
      lsm6ds3tr_c_acceleration_raw_get(&_dev_ctx, data_raw);

      _self_test_acc_task.SetWakeTime(10);
    }
    do {
      lsm6ds3tr_c_xl_flag_data_ready_get(&_dev_ctx, &_rx_buf);
    } while (!_rx_buf);

    lsm6ds3tr_c_acceleration_raw_get(&_dev_ctx, data_raw);

    for (uint8_t k = 0; k < 3; k++) {
      avg_values[i / 5][k] += 0.2 * lsm6ds3tr_c_from_fs4g_to_mg(data_raw[k]);
    }

    // enable acc self test after obtain OUT_NOST
    if (i == 4) {
      lsm6ds3tr_c_xl_self_test_set(&_dev_ctx, LSM6DS3TR_C_XL_ST_POSITIVE);
      _self_test_acc_task.SetWakeTime(100);
    }
    scheduler.Queue(&_self_test_acc_task, nullptr);
    i++;
    return;
  }

  float_t test_val[3];
  for (uint8_t i = 0; i < 3; i++) {
    test_val[i] = fabsf((avg_values[0][i] - avg_values[1][i]));
  }
  constexpr float_t MIN_ST_LIMIT_mg = 90.0f;
  constexpr float_t MAX_ST_LIMIT_mg = 1700.0f;
  // check self test limit
  bool pass = true;
  for (uint8_t i = 0; i < 3; i++) {
    if ((MIN_ST_LIMIT_mg > test_val[i]) || (test_val[i] > MAX_ST_LIMIT_mg)) {
      pass = false;
    }
  }

  // disable acc sensor and self test
  lsm6ds3tr_c_xl_self_test_set(&_dev_ctx, LSM6DS3TR_C_XL_ST_DISABLE);
  lsm6ds3tr_c_xl_data_rate_set(&_dev_ctx, LSM6DS3TR_C_XL_ODR_OFF);
  if (callback) {
    callback(nullptr, reinterpret_cast<void *>(pass));
  }
}

void ImuService::GyroSelfTest(void *cb) {
  static callback_t callback = reinterpret_cast<callback_t>(cb);

  static int16_t data_raw[3];
  // avg_values[0]: OUT_NOST, avg_values[1]: OUT_ST
  static float_t avg_values[2][3];
  static uint8_t i;

  if (_state == State::IDLE) {
    _state = State::SELF_TEST_GYRO;
    scheduler.DisablePeriodic(&_routine_task);
    SwReset();
    // initialize sensor, set FS=2000dps, ODR=208Hz, BDU=1
    lsm6ds3tr_c_gy_full_scale_set(&_dev_ctx, LSM6DS3TR_C_2000dps);
    lsm6ds3tr_c_gy_data_rate_set(&_dev_ctx, LSM6DS3TR_C_GY_ODR_208Hz);
    lsm6ds3tr_c_block_data_update_set(&_dev_ctx, PROPERTY_ENABLE);

    // wait 150ms for stable output
    _self_test_gyro_task.SetWakeTime(150);
    scheduler.Queue(&_self_test_gyro_task, nullptr);
    memset(avg_values, 0, 2 * 3 * sizeof(float_t));
    i = 0;
    return;
  }

  if (i < 10) {
    if (i == 0 || i == 5) {
      // check gyro data ready bit, wait for first sample
      do {
        lsm6ds3tr_c_gy_flag_data_ready_get(&_dev_ctx, &_rx_buf);
      } while (!_rx_buf);

      // read and discard data
      lsm6ds3tr_c_angular_rate_raw_get(&_dev_ctx, data_raw);

      _self_test_gyro_task.SetWakeTime(10);
    }
    do {
      lsm6ds3tr_c_gy_flag_data_ready_get(&_dev_ctx, &_rx_buf);
    } while (!_rx_buf);

    lsm6ds3tr_c_angular_rate_raw_get(&_dev_ctx, data_raw);

    for (uint8_t k = 0; k < 3; k++) {
      avg_values[i / 5][k] +=
          0.2f * lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw[k]);
    }

    // enable gyro self test after obtain OUT_NOST
    if (i == 4) {
      lsm6ds3tr_c_gy_self_test_set(&_dev_ctx, LSM6DS3TR_C_GY_ST_POSITIVE);
      _self_test_gyro_task.SetWakeTime(50);
    }
    scheduler.Queue(&_self_test_gyro_task, nullptr);
    i++;
    return;
  }

  float_t test_val[3];
  for (uint8_t j = 0; j < 3; j++) {
    test_val[j] = fabsf((avg_values[0][j] - avg_values[1][j]));
  }
  constexpr float_t MIN_ST_LIMIT_mdps = 150000.0f;
  constexpr float_t MAX_ST_LIMIT_mdps = 700000.0f;
  // check self test limit
  bool pass = true;
  for (uint8_t j = 0; j < 3; j++) {
    if ((MIN_ST_LIMIT_mdps > test_val[j]) ||
        (test_val[j] > MAX_ST_LIMIT_mdps)) {
      pass = false;
    }
  }

  // disable gyro sensor and self test
  lsm6ds3tr_c_gy_self_test_set(&_dev_ctx, LSM6DS3TR_C_GY_ST_DISABLE);
  lsm6ds3tr_c_gy_data_rate_set(&_dev_ctx, LSM6DS3TR_C_GY_ODR_OFF);
  if (callback) {
    callback(nullptr, reinterpret_cast<void *>(pass));
  }
}

void ImuService::SwReset() {
  // Restore default configuration
  lsm6ds3tr_c_reset_set(&_dev_ctx, PROPERTY_ENABLE);
  do {
    lsm6ds3tr_c_reset_get(&_dev_ctx, &_rx_buf);
  } while (_rx_buf);
}
uint8_t read_record[20] = {0};
uint8_t write_record[20] = {0};
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len) {
  static int count = 0;
  if (count == 20) count = 0;
  write_record[count++] = reg;
  write_record[count++] = bufp[0];
  I2C_HandleTypeDef *hi2c = reinterpret_cast<I2C_HandleTypeDef *>(handle);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);

  // while (!g_imu_service.tx_ready);
  // g_imu_service.tx_ready = false;

  if (hi2c->ErrorCode) {
    MX_I2C1_Init();
  }
  int32_t status =
      HAL_I2C_Mem_Write(hi2c, LSM6DS3TR_C_I2C_ADD_L, reg, I2C_MEMADD_SIZE_8BIT,
                        (uint8_t *)bufp, len, 1000);

  if (hi2c->ErrorCode) {
    MX_I2C1_Init();
  }
  // while (!g_imu_service.tx_ready);
  //  g_imu_service.tx_ready = false;
  //  HAL_I2C_Master_Transmit_IT(hi2c, 0xD4, &reg, 1);
  //  while (!g_imu_service.tx_ready);
  //  g_imu_service.tx_ready = false;
  //  int32_t status =
  //      HAL_I2C_Master_Transmit_IT(hi2c, 0XD4, const_cast<uint8_t *>(bufp),
  //      len);
  //  while (!g_imu_service.tx_ready);
  return status;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len) {
  static int count = 0;
  if (count == 20) count = 0;
  read_record[count++] = reg;
  I2C_HandleTypeDef *hi2c = reinterpret_cast<I2C_HandleTypeDef *>(handle);

  // while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);
  // while (!g_imu_service.rx_ready);

  // g_imu_service.rx_ready = false;
  if (hi2c->ErrorCode) {
    MX_I2C1_Init();
  }
  int32_t status = HAL_I2C_Mem_Read(hi2c, LSM6DS3TR_C_I2C_ADD_L, reg,
                                    I2C_MEMADD_SIZE_8BIT, bufp, len, 10000);

  if (hi2c->ErrorCode) {
    MX_I2C1_Init();
  }
  // while (!g_imu_service.rx_ready);
  //  while (!g_imu_service.tx_ready);
  //  g_imu_service.tx_ready = false;
  //  HAL_I2C_Master_Transmit_IT(hi2c, 0xD4, &reg, 1);
  //  while (!g_imu_service.tx_ready);
  //  while (!g_imu_service.rx_ready);
  //  g_imu_service.rx_ready = false;
  //  int32_t status = HAL_I2C_Master_Receive_IT(hi2c, 0XD5, bufp, len);
  //  while (!g_imu_service.rx_ready);

  return status;
}

static void platform_delay(uint32_t ms) { HAL_Delay(ms); }

}  // namespace hitcon
