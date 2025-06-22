#ifndef HITCON_LOGIC_IMU_LOGIC_H_
#define HITCON_LOGIC_IMU_LOGIC_H_

#include <Service/ImuService.h>
#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>

#include <cmath>

using namespace hitcon::service::sched;

namespace hitcon {

namespace {
enum class RoutineState {
  WAIT_15,  // wait for 15 ms for stable
  INIT,
  IDLE,
  GET_STEP,
  ST_GYRO,
  ST_ACC,
};

enum class InitState {
  CHECK_ID,
  WAIT_ID,
  SW_RESET,
  WAIT_SW_RESET,
  CONFIGURE,
  WAIT_CONFIGURE,
  DONE
};

#define WAIT_TIME_GYRO_A 150
#define WAIT_TIME_GYRO_B 50
#define WAIT_TIME_ACC_A 100
#define WAIT_TIME_ACC_B 50

enum class SelfTestState {
  SW_RESET,
  WAIT_SW_RESET,
  CONFIGURE,
  WAIT_CONFIGURE,
  WAIT_A,  // GYRO: 150ms, ACC: 100ms
  // repeat 6 times and discard first
  CHECK_DA,  // data available
  WAIT_DA,
  READ_OUT,
  WAIT_READ_OUT,
  ENABLE_SELF_TEST,
  WAIT_ENABLE_ST,
  WAIT_B,  // GYRO: 50ms, ACC: 100ms, then go back to CHECK_DA * 6
  VERIFY,
  DONE
};

constexpr float_t MIN_ST_LIMIT_mg = 90.0f;
constexpr float_t MAX_ST_LIMIT_mg = 1700.0f;
constexpr float_t MIN_ST_LIMIT_mdps = 150000.0f;
constexpr float_t MAX_ST_LIMIT_mdps = 700000.0f;
}  // namespace

class ImuLogic {
 public:
  ImuLogic();
  void Init();
  void GyroSelfTest(callback_t cb, void *cb_arg1);
  void AccSelfTest(callback_t cb, void *cb_arg1);

 private:
  PeriodicTask _routine_task;
  uint8_t _buf[6], _count;
  float_t _avg_values[2][3];
  RoutineState _state;
  InitState _init_state;
  SelfTestState _self_test_state;
  callback_t _gyro_st_cb, _acc_st_cb;
  void *_gyro_st_cb_arg1, *_acc_st_cb_arg1;
  uint32_t _start_time;

  void OnRxDone(void *arg1);
  void OnTxDone(void *arg1);
  void Routine(void *arg1);
};

extern ImuLogic g_imu_logic;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IMU_LOGIC_H_
