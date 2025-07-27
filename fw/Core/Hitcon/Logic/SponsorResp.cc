#include "Logic/SponsorResp.h"

#include "Logic/XBoardLogic.h"
#include "Logic/XBoardRecvFn.h"

using namespace hitcon::service::xboard;

namespace hitcon {
namespace sponsor {

SponsorResp::SponsorResp() {}

void SponsorResp::Init() {
  g_xboard_logic.SetOnPacketArrive(
      (callback_t)&SponsorResp::XBoardRequestHandler, this, SPONSOR_REQ_ID);
}

void SponsorResp::XBoardRequestHandler(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
}

SponsorResp g_sponsor_resp;

}  // namespace sponsor
}  // namespace hitcon
