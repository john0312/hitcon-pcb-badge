#ifndef HITCON_SERVICE_IMU_SERVICE_H_
#define HITCON_SERVICE_IMU_SERVICE_H_

extern "C" {
#include <Service/Imu/lsm6ds3tr-c_reg.h>
}

#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>

#include "gpio.h"
#include "i2c.h"

#define I2C_HANDLE hi2c1

using namespace hitcon::service::sched;

namespace hitcon {

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);

class ImuService {
 public:
  enum class State {
    INIT,
    IDLE,
    SELF_TEST_GYRO,
    SELF_TEST_ACC,
  };

  ImuService();

  // 1. register I2C Rx interrupt callback
  // 2. queue init task
  // 3. wait 15ms
  void Init();

  /**
   * @brief Set callback for GetGyro() and GetAcc()
   * @param callback will be called when Gyro/Acc data is ready, it will pass
   * the request_id, i.g. callback(arg1, request_id)
   * @param arg1 if callback is a method, pass "this" of the class
   */
  // void SetOnRecvData(callback_t callback, void *arg1);

  /**
   * @brief async get gyroscope data
   *
   * @return uint32_t request_id, when () is called
   */
  // uint32_t GetGyro(uint8_t length);

  /**
   * @brief Get the Acc object
   *
   * @return uint8_t
   */
  // uint32_t GetAcc(uint8_t length);
  void GetPedometer();

  void GyroSelfTest(void *cb);
  void AccSelfTest(void *cb);

  volatile bool rx_ready, tx_ready;

 private:
  stmdev_ctx_t _dev_ctx;
  DelayedTask _init_task, _self_test_gyro_task, _self_test_acc_task;
  PeriodicTask _routine_task;
  uint8_t _rx_buf;
  uint8_t _queue_index;
  State _state;

  // init LSM6DS3 control reg
  // TODO: determine ODR,
  void LSM6DS3Init(void *arg);
  // query sensor reg value
  void QueueReadReg(uint8_t addr);
  // polling read reg value
  uint8_t ReadReg(uint8_t addr);
  // write value to reg
  void WriteReg(uint8_t addr, uint8_t value);
  // callback for QueueReadReg()
  void OnRecvReg(void *arg1);
  void Routine(void *arg1);
  // software reset (blocking), restore default configuration
  void SwReset();
};

extern ImuService g_imu_service;

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_I2C_SERVICE_H_
