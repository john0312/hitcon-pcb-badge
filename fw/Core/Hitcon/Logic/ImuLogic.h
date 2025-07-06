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
  ST_GYRO,
  ST_ACC,
  IDLE,
  GET_STEP,
  WAIT_STEP,
};

enum class InitState {
  CHECK_ID,
  WAIT_ID,
  SW_RESET,
  WAIT_SW_RESET,
  RESET_COUNT,
  WAIT_RESET_COUNT,
  CONFIGURE,
  WAIT_CONFIGURE,
  DONE
};

constexpr uint32_t WAIT_TIME_GYRO_A = 150;
constexpr uint32_t WAIT_TIME_GYRO_B = 50;
constexpr uint32_t WAIT_TIME_ACC_A = 100;
constexpr uint32_t WAIT_TIME_ACC_B = 50;

constexpr float_t MIN_ST_LIMIT_mg = 90.0f;
constexpr float_t MAX_ST_LIMIT_mg = 1700.0f;
constexpr float_t MIN_ST_LIMIT_mdps = 150000.0f;
constexpr float_t MAX_ST_LIMIT_mdps = 700000.0f;

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

constexpr uint32_t ROUTINE_INTERVAL = 500;  // milisecond
constexpr uint32_t SHAKING_THRESHOLD = 5;
constexpr uint32_t PROXIMITY_INTERVAL = 3 * 60 * 1000;  // 3 minutes
// step count within PROXIMITY_INTERVAL will be divided by this factor then call
// GameController SendProximity
constexpr uint8_t SCALE_FACTOR = 4;
}  // namespace

class ImuLogic {
 public:
  ImuLogic();
  void Init();
  void GyroSelfTest(callback_t cb, void *cb_arg1);
  void AccSelfTest(callback_t cb, void *cb_arg1);

  // return the step count since last init
  // update every ROUTINE_INTERVAL
  uint16_t GetStep() { return _step; }

  // within ROUTINE_INTERVAL detected step count if larger than
  // SHAKING_THRESHOLD then is considered shaking
  // update every ROUTINE_INTERVAL
  bool IsShaking() { return _is_shaking; }

 private:
  PeriodicTask _routine_task, _proximity_task;
  uint8_t _buf[6], _count;
  float_t _avg_values[2][3];
  RoutineState _state;
  InitState _init_state;
  SelfTestState _self_test_state;
  callback_t _gyro_st_cb, _acc_st_cb;
  void *_gyro_st_cb_arg1, *_acc_st_cb_arg1;
  uint32_t _start_time;
  uint16_t _step;
  bool _is_shaking;

  void OnRxDone(void *arg1);
  void OnTxDone(void *arg1);
  void Routine(void *arg1);

  // send proximity packet every PROXIMITY_INTERVAL
  void ProximityRoutine(void *arg);
};

extern ImuLogic g_imu_logic;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IMU_LOGIC_H_
