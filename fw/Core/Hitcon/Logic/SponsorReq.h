#ifndef HITCON_LOGIC_SPONSOR_REQ_H_
#define HITCON_LOGIC_SPONSOR_REQ_H_

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

#endif  // HITCON_LOGIC_SPONSOR_REQ_H_
