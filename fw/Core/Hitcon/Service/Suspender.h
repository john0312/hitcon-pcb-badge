#ifndef HITCON_SERVICE_SUSPENDER_H_
#define HITCON_SERVICE_SUSPENDER_H_

#include <Service/Sched/SysTimer.h>
#include <stdint.h>

namespace hitcon {

// In charge of suspending interrupts for longer task.
class Suspender {
 public:
  Suspender();

  // Locks or unlocks a blocker to suspending.
  void IncBlocker();
  void DecBlocker();

  bool TrySuspend();
  bool TryResume();

  bool IsSuspended() { return suspended_; }

 private:
  int blockers_;

#ifdef DEBUG
  uint32_t last_suspend_ = 0;
  uint32_t last_resume_ = 0;
#endif
  bool suspended_;
};

extern Suspender g_suspender;

}  // namespace hitcon

#endif  // HITCON_SERVICE_SUSPENDER_H_
