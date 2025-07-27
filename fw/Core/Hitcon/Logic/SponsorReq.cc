#include "Logic/SponsorReq.h"

#include "Logic/GameController.h"
#include "Logic/XBoardLogic.h"
#include "Logic/XBoardRecvFn.h"

using namespace hitcon::game;
using namespace hitcon::service::xboard;
using namespace hitcon::ir;

namespace hitcon {
namespace sponsor {

SponsorReq::SponsorReq() {}

void SponsorReq::Init() {
  g_xboard_logic.SetOnPacketArrive(
      (callback_t)&SponsorReq::XBoardResponseHandler, this, SPONSOR_RESP_ID);
}

void SponsorReq::OnXBoardConnect() {
  g_xboard_logic.QueueDataForTx(g_game_controller.GetUsername(),
                                IR_USERNAME_LEN, SPONSOR_REQ_ID);
}

void SponsorReq::XBoardResponseHandler(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
}

SponsorReq g_sponsor_req;

}  // namespace sponsor
}  // namespace hitcon
