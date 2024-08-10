#ifndef HITCON_SERVICE_SUSPENDER_H_
#define HITCON_SERVICE_SUSPENDER_H_

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

  bool suspended_;
};

extern Suspender g_suspender;

}  // namespace hitcon

#endif  // HITCON_SERVICE_SUSPENDER_H_
