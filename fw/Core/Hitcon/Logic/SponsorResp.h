#ifndef HITCON_LOGIC_SPONSOR_RESP_H_
#define HITCON_LOGIC_SPONSOR_RESP_H_
#include <Hitcon.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_SPONSOR

namespace hitcon {
namespace sponsor {

class SponsorResp {
 public:
  SponsorResp();

  void Init();

 private:
  void XBoardRequestHandler(void *arg);
};

extern SponsorResp g_sponsor_resp;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_SPONSOR

#endif  // HITCON_LOGIC_SPONSOR_RESP_H_
