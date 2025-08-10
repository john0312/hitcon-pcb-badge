#ifndef HITCON_SERVICE_IMU_SERVICE_H_
#define HITCON_SERVICE_IMU_SERVICE_H_

#include <Service/Sched/Scheduler.h>
#include <Util/CircularQueue.h>
#include <Util/callback.h>

#include "i2c.h"
#include "stm32f1xx_hal_i2c.h"

using namespace hitcon::service::sched;

#define I2C_HANDLE &hi2c1
#define SLAVE_ADDR 0xD5U
#define QUEUE_SIZE 10
// if I2C rx/tx interrupt is not called within TIMEOUT ms then
// reset IMU service and logic
#define TIMEOUT 10000

namespace hitcon {

// handle I2C communication
class ImuService {
 public:
  enum class State {
    INIT,
    WAIT_POWEROFF,  // IMU_PWR is high so power is off
    WAIT_POWERON,   // IMU_PWR is low so power is on, we need to wait for it to
                    // init.
    IDLE,
    READING,
    WRITING,
  };

  struct ReadOp {
    uint8_t addr;
    uint8_t* value_ptr;
  };

  struct WriteOp {
    uint8_t addr;
    uint8_t value;
  };

  ImuService();

  void Init();
  void ResetI2C();

  // query sensor reg value
  void QueueReadReg(uint8_t addr, uint8_t* value);
  // write value to reg
  void QueueWriteReg(uint8_t addr, uint8_t value);

  // write struct value to reg (type-safe version)
  template <typename T>
  void QueueWriteReg(uint8_t addr, const T& value) {
    static_assert(sizeof(T) == 1UL, "Register value must be 1 byte");
    QueueWriteReg(addr, *reinterpret_cast<const uint8_t*>(&value));
  }

  // the callback will be called when all read operations are done
  void SetRxCallback(callback_t callback, void* callback_arg1);
  // the callback will be called when all write operations are done
  void SetTxCallback(callback_t callback, void* callback_arg1);
  void I2CCallback(void* arg2);

  volatile State state = State::INIT;
  bool IsBusy() { return state != State::IDLE; };
  Task interrupt_task;

 private:
  CircularQueue<ReadOp, QUEUE_SIZE> _rx_queue;
  CircularQueue<WriteOp, QUEUE_SIZE> _tx_queue;
  callback_t _rx_cb = nullptr, _tx_cb = nullptr;
  void *_rx_cb_arg1 = nullptr, *_tx_cb_arg1 = nullptr;
  bool _is_rx_done, _is_tx_done;

  int i2c_busy_cnt = 0;

  PeriodicTask _routine_task;
  void Routine(void* arg);

  bool IsI2CIdle();
};

extern ImuService g_imu_service;

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_I2C_SERVICE_H_
