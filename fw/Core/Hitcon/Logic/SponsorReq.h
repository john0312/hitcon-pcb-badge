#ifndef HITCON_LOGIC_SPONSOR_REQ_H_
#define HITCON_LOGIC_SPONSOR_REQ_H_
#include <Hitcon.h>

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

 private:
  void XBoardResponseHandler(void *arg);
};

extern SponsorReq g_sponsor_req;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_ATTENDEE

#endif  // HITCON_LOGIC_SPONSOR_REQ_H_
