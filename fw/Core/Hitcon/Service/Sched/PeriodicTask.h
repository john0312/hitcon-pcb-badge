/*
 * PeriodicTask.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_PERIODICTASK_H_
#define HITCON_SERVICE_SCHED_PERIODICTASK_H_

#include "DelayedTask.h"

namespace hitcon {
namespace service {
namespace sched {

class PeriodicTask : public DelayedTask {
 private:
  bool enabled;
  unsigned interval;
  void *savedThisptr;
  task_callback_t savedCallback;
  void AutoRequeueCb(void *arg);

 public:
  // For prio, see Scheduler.h
  constexpr PeriodicTask(unsigned prio, task_callback_t callback, void *thisptr,
                         unsigned interval)
      : DelayedTask(prio, (task_callback_t)&PeriodicTask::AutoRequeueCb,
                    (void *)this, 0),
        enabled(false), interval(interval), savedThisptr(thisptr),
        savedCallback(callback) {}

  virtual ~PeriodicTask();
  void Enable();
  void Disable();
  bool IsEnabled() { return enabled; }
};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_PERIODICTASK_H_ */
