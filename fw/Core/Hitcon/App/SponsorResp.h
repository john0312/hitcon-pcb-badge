#ifndef HITCON_APP_SPONSOR_RESP_H_
#define HITCON_APP_SPONSOR_RESP_H_
#include <App/app.h>
#include <Hitcon.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_SPONSOR

namespace hitcon {
namespace sponsor {

constexpr uint8_t kSponsorIdMask = 0x1f;
constexpr uint8_t kSponsorNonceMask = 0x3f;
constexpr size_t kSponsorPrefixLen = 4;
constexpr size_t kSponsorXbExtraData = 2;

class SponsorResp : public ::hitcon::App {
 public:
  SponsorResp();

  void Init();

  void OnPeerConnect();
  void OnPeerDisconnect();

  void OnEntry() override;
  void OnExit() override { state_ = 1; };
  void OnButton(button_t button) override {
    // No action needed.
  };

 private:
  int state_;
  // 0 - Not connected.
  // 1 - Connected waiting for peer's username.
  // 2 - Waiting for our username.
  // 3 - Waiting for signing slot.
  // 4 - Waiting for signing
  // 5 -

  uint8_t sign_buffer_[kSponsorPrefixLen + hitcon::ir::IR_USERNAME_LEN];

  hitcon::service::sched::DelayedTask main_task_;

  uint8_t xb_packet_buffer[kSponsorXbExtraData + ECC_SIGNATURE_SIZE];
  bool main_task_scheduled_;

  bool consented_;

  bool PopulateSignBuffer();
  void XBoardRequestHandler(void *arg);
  void XBoardAckHandler(void *arg);
  void EnsureRoutineQueued();
  void RoutineTask(void *unused);
  void RoutineTaskInternal();
  void OnSignFinish(hitcon::ecc::Signature *signature);
};

extern SponsorResp g_sponsor_resp;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_SPONSOR

#endif  // HITCON_APP_SPONSOR_RESP_H_
