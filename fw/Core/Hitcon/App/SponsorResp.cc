#include <App/SponsorResp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>
#include <string.h>

using namespace hitcon::service::xboard;
using namespace hitcon::service::sched;

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_SPONSOR

namespace hitcon {
namespace sponsor {

constexpr size_t kSponsorRespTaskInterval = 500;

SponsorResp::SponsorResp()
    : main_task_(850, (task_callback_t)&SponsorResp::RoutineTask, this,
                 kSponsorRespTaskInterval),
      main_task_scheduled_(false) {}

void SponsorResp::Init() {
  g_xboard_logic.SetOnPacketArrive(
      (callback_t)&SponsorResp::XBoardRequestHandler, this, SPONSOR_REQ_ID);
  g_xboard_logic.SetOnPacketArrive((callback_t)&SponsorResp::XBoardAckHandler,
                                   this, SPONSOR_RESP_ACK);
}

void SponsorResp::OnEntry() {
  consented_ = true;
  display_set_mode_scroll_text("Preparing...");
}

void SponsorResp::OnPeerConnect() {
  state_ = 1;
  consented_ = false;
}
void SponsorResp::OnPeerDisconnect() { state_ = 0; }

bool SponsorResp::PopulateSignBuffer() {
  // Take care to avoid colliding with the usual IR packets.
  const uint8_t* ptr = hitcon::g_game_controller.GetUsername();
  if (!ptr) return false;
  uint8_t sponsor_id = ptr[hitcon::ir::IR_USERNAME_LEN - 1] & kSponsorIdMask;
  uint8_t nonce = g_fast_random_pool.GetRandom() & kSponsorNonceMask;
  sign_buffer_[0] = 0x00;
  sign_buffer_[1] = static_cast<uint8_t>(packet_type::kSponsorActivity);
  sign_buffer_[2] = sponsor_id;
  sign_buffer_[3] = nonce;
  xb_packet_buffer[0] = sponsor_id;
  xb_packet_buffer[1] = nonce;
  return true;
}

void SponsorResp::XBoardRequestHandler(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  if (packet->len == hitcon::ir::IR_USERNAME_LEN) {
    if (state_ == 1) {
      memcpy(&sign_buffer_[kSponsorPrefixLen], packet->data,
             hitcon::ir::IR_USERNAME_LEN);
      state_ = 2;
      EnsureRoutineQueued();
    }
  }
}

void SponsorResp::XBoardAckHandler(void* arg) {
  PacketCallbackArg* packet = reinterpret_cast<PacketCallbackArg*>(arg);
  if (packet->len == 2) {
    if (packet->data[0] == xb_packet_buffer[0] &&
        packet->data[1] == xb_packet_buffer[1]) {
      if (state_ >= 6 && state_ < 11) {
        state_ = 11;
      }
    }
  }
}

void SponsorResp::EnsureRoutineQueued() {
  if (state_ != 0 && !main_task_scheduled_) {
    main_task_.SetWakeTime(SysTimer::GetTime() + kSponsorRespTaskInterval);
    scheduler.Queue(&main_task_, nullptr);
    main_task_scheduled_ = true;
  }
}

void SponsorResp::RoutineTask(void* unused) {
  main_task_scheduled_ = false;
  if (state_ == 0) return;
  RoutineTaskInternal();
  EnsureRoutineQueued();
}

void SponsorResp::RoutineTaskInternal() {
  if (state_ == 2) {
    bool ret = PopulateSignBuffer();
    if (ret) {
      state_ = 3;
    }
    return;
  }

  if (state_ == 3) {
    // See if we've a signing slot.
    bool ret = hitcon::ecc::g_ec_logic.StartSign(
        sign_buffer_, sizeof(sign_buffer_),
        (task_callback_t)&SponsorResp::OnSignFinish, this);
    if (ret) {
      state_ = 4;
    }
    return;
  }

  if (state_ == 4) {
    // Waiting for sign to finish.
    return;
  }

  if (state_ == 5) {
    // Signing finished, see if we've consent.
    if (consented_) state_ = 6;
    return;
  }

  if (state_ == 6) {
    // Got consent, sending...
    g_xboard_logic.QueueDataForTx(xb_packet_buffer, sizeof(xb_packet_buffer),
                                  SPONSOR_RESP_ID);
    display_set_mode_scroll_text("Sending...");
    state_++;
    return;
  }

  if (state_ > 6 && state_ < 10) {
    // Nothing, waiting for the message to show.
    state_++;
    return;
  }

  if (state_ == 10) {
    // Didn't receive? Retry.
    state_ = 6;
  }

  if (state_ == 11) {
    display_set_mode_scroll_text("Sent");
    state_++;
    return;
  }

  if (state_ > 11 && state_ <= 24) {
    // Nothing, waiting for the message to show.
    state_++;
    return;
  }

  if (state_ == 25) {
    state_++;
    hitcon::badge_controller.BackToMenu(this);
    return;
  }
}

void SponsorResp::OnSignFinish(hitcon::ecc::Signature* signature) {
  if (state_ == 4) {
    signature->toBuffer(&xb_packet_buffer[kSponsorXbExtraData]);
    state_ = 5;
  }
}

SponsorResp g_sponsor_resp;

}  // namespace sponsor
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_SPONSOR
