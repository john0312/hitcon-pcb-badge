#include <Service/Sched/Checks.h>
#include <Service/Suspender.h>

using hitcon::service::sched::my_assert;

namespace hitcon {

Suspender g_suspender;

Suspender::Suspender() : blockers_(0), suspended_(false) {}

void Suspender::IncBlocker() { blockers_++; }

void Suspender::DecBlocker() {
  my_assert(blockers_ > 0);
  blockers_--;
}

bool Suspender::TrySuspend() {
  if (blockers_ != 0) {
    return false;
  }
  my_assert(suspended_ == false);
  suspended_ = true;
#ifdef DEBUG
  last_suspend_ = hitcon::service::sched::SysTimer::GetTime();
#endif
  return true;
}

bool Suspender::TryResume() {
  my_assert(suspended_ == true);
  suspended_ = false;
#ifdef DEBUG
  last_resume_ = hitcon::service::sched::SysTimer::GetTime();
#endif
  return true;
}

}  // namespace hitcon
