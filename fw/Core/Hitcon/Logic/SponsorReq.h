#ifndef HITCON_LOGIC_SPONSOR_REQ_H_
#define HITCON_LOGIC_SPONSOR_REQ_H_
#include <Hitcon.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_ATTENDEE

namespace hitcon {
namespace sponsor {

class SponsorReq {
 public:
  SponsorReq();

  void Init();
  void OnXBoardConnect();
  void OnXBoardDisconnect();

 private:
  int state_;
  hitcon::service::sched::DelayedTask main_task_;
  hitcon::ir::IrData ir_data_;
  bool pending_send_;

  bool main_task_scheduled_;

  void EnsureQueued();
  bool TrySendUsername();
  void RoutineTask(void* unused);
  bool RoutineTaskInternal();
  void XBoardResponseHandler(void* arg);
  bool TrySend();
};

extern SponsorReq g_sponsor_req;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_ATTENDEE

#endif  // HITCON_LOGIC_SPONSOR_REQ_H_
