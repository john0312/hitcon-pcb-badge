#ifndef HITCON_LOGIC_SPONSOR_RESP_H_
#define HITCON_LOGIC_SPONSOR_RESP_H_

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

#endif  // HITCON_LOGIC_SPONSOR_RESP_H_
