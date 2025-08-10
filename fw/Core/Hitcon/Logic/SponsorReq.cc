#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/RandomPool.h>
#include <Logic/SponsorReq.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>

using namespace hitcon::game;
using namespace hitcon::service::xboard;
using namespace hitcon::service::sched;
using namespace hitcon::ir;

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_ATTENDEE

namespace hitcon {
namespace sponsor {

constexpr int kSponsorReqTaskInterval = 500;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
SponsorReq::SponsorReq()
    : main_task_(850, (task_callback_t)&SponsorReq::RoutineTask, this,
                 kSponsorReqTaskInterval),
      main_task_scheduled_(false) {}
#pragma GCC diagnostic pop

void SponsorReq::Init() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_xboard_logic.SetOnPacketArrive(
      (callback_t)&SponsorReq::XBoardResponseHandler, this, SPONSOR_RESP_ID);
#pragma GCC diagnostic pop
}

void SponsorReq::OnXBoardConnect() {
  state_ = 1;
  EnsureQueued();
}

void SponsorReq::OnXBoardDisconnect() { state_ = 0; }

bool SponsorReq::TrySendUsername() {
  const uint8_t* ptr = g_game_controller.GetUsername();
  if (!ptr) return false;
  g_xboard_logic.QueueDataForTx(ptr, IR_USERNAME_LEN, SPONSOR_REQ_ID);
  memcpy(&ir_data_.opaq.sponsor_activity.user[0], ptr, IR_USERNAME_LEN);
  return true;
}

void SponsorReq::XBoardResponseHandler(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  if (packet->len == ECC_SIGNATURE_SIZE + 2) {
    uint8_t buff[2];
    buff[0] = packet->data[0];
    buff[1] = packet->data[1];
    g_xboard_logic.QueueDataForTx(buff, sizeof(buff), SPONSOR_RESP_ACK);

    if (state_ == 4) {
      ir_data_.ttl = 0;
      ir_data_.type = packet_type::kSponsorActivity;
      ir_data_.opaq.sponsor_activity.sponsor_id = packet->data[0];
      ir_data_.opaq.sponsor_activity.nonce = packet->data[1];
      memcpy(&ir_data_.opaq.sponsor_activity.sig[0], &packet->data[2],
             ECC_SIGNATURE_SIZE);
      pending_send_ = true;

      state_ = 5;
      EnsureQueued();
    }
  }
}

void SponsorReq::EnsureQueued() {
  if (!main_task_scheduled_) {
    main_task_.SetWakeTime(SysTimer::GetTime() + kSponsorReqTaskInterval);
    scheduler.Queue(&main_task_, nullptr);
    main_task_scheduled_ = true;
  }
}
void SponsorReq::RoutineTask(void* unused) {
  main_task_scheduled_ = false;
  bool ret = RoutineTaskInternal();
  if (ret) {
    EnsureQueued();
  }
}

bool SponsorReq::RoutineTaskInternal() {
  bool to_queue = false;
  if (state_ == 1 || state_ == 2 || state_ == 3) {
    bool ret = TrySendUsername();
    if (ret) state_++;
    to_queue = true;
  } else if (state_ == 4) {
    // Just waiting for the packet.
  } else if (state_ == 5) {
    display_set_mode_scroll_text("Received");
    state_++;
    to_queue = true;
  } else if (state_ >= 6 && state_ < 19) {
    // Waiting to clear out the text.
    state_++;
    to_queue = true;
  } else if (state_ == 19) {
    // Clear out the text.
    hitcon::badge_controller.BackToMenu(hitcon::badge_controller.current_app);
    state_++;
  } else if (state_ == 20) {
    // Finished.
  }
  if (pending_send_) {
    TrySend();
  }
  if (pending_send_) {
    to_queue = true;
  }
  return to_queue;
}

bool SponsorReq::TrySend() {
  if (!pending_send_) return true;
  bool ret = hitcon::ir::irController.SendPacketWithRetransmit(
      reinterpret_cast<uint8_t*>(&ir_data_),
      ir::IR_DATA_HEADER_SIZE + sizeof(struct SponsorActivityPacket), 3,
      ::hitcon::ir::AckTag::ACK_TAG_NONE);
  if (ret) {
    pending_send_ = false;
  }
  return ret;
}

SponsorReq g_sponsor_req;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_ATTENDEE
