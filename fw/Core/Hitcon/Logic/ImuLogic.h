#ifndef HITCON_LOGIC_IMU_LOGIC_H_
#define HITCON_LOGIC_IMU_LOGIC_H_

#include "Service/Sched/Scheduler.h"
#include "i2c.h"

namespace hitcon {

enum imu_event_t {
  CHECK_WHOAMI,
  GET_PEDOMETER,

};

struct step_data {
  size_t count;
  size_t timestamp;
};

class ImuLogic {
 public:
 private:
};

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IMU_LOGIC_H_
