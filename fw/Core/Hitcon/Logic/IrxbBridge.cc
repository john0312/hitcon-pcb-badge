#include <App/ConnectMenuApp.h>
#include <Logic/Display/display.h>
#include <Logic/EcLogic.h>
#include <Logic/GameController.h>
#include <Logic/IrLogic.h>
#include <Logic/IrxbBridge.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Util/uint_to_str.h>

#include <algorithm>

using hitcon::service::sched::SysTimer;
using namespace hitcon::ir_xb_bridge;
using hitcon::ecc::g_ec_logic;

namespace hitcon {

namespace {
constexpr unsigned kIrxbDelayTime = 100;
constexpr int kIrxbShowCyclesStart = 60;
constexpr int kIrxbShowCyclesExtra = 70;
constexpr unsigned kStateBase = 32;
constexpr unsigned kStateTxEnd = kStateBase + 16 * ir::RETX_QUEUE_SIZE;
constexpr unsigned kStateMenu = 2;
constexpr unsigned kStateInit = 1;
}  // namespace

IrxbBridge g_irxb_bridge;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
IrxbBridge::IrxbBridge()
    : routine_task_(950, (callback_t)&IrxbBridge::RoutineTask, this,
                    kIrxbDelayTime),
      state_(0) {}
#pragma GCC diagnostic pop

void IrxbBridge::Init() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  service::xboard::g_xboard_logic.SetOnPacketArrive(
      (callback_t)&IrxbBridge::OnPacketReceived, this,
      service::xboard::RecvFnId::IR_TO_ATTENDEE);
#pragma GCC diagnostic pop
}

void IrxbBridge::OnXBoardBasestnConnect() {
  state_ = 1;
  show_cycles_ = kIrxbShowCyclesStart;
  tx_cnt_ = rx_cnt_ = 0;

  tama_state_ = TamaState::kTamaStateInit;
  score_state_ = ScoreState::kScoreStateInit;
  tama_app.ResetRestorePacketPoll();
  show_name_app.ResetSetScorePacketPoll();

  EnsureRoutineQueued();
}

void IrxbBridge::OnXBoardBasestnDisconnect() { state_ = 0; }

void IrxbBridge::RoutineTask() {
  routine_queued_ = false;
  bool ret = RoutineInternal();
  if (!ret) return;
  EnsureRoutineQueued();
}

void IrxbBridge::EnsureRoutineQueued() {
  if (!routine_queued_) {
    routine_task_.SetWakeTime(SysTimer::GetTime() + kIrxbDelayTime);
    service::sched::scheduler.Queue(&routine_task_, nullptr);
  }
}
bool IrxbBridge::RoutineInternal() {
  if (state_ == 0) return false;

  if (state_ == kStateMenu) return true;

  bool show_text = false;
  bool set_txrx_text = false;
  if (state_ == kStateInit) {
    // Start of the loop
    state_ = kStateBase;
    disp_txt_[0] = '-';
    disp_txt_[1] = '-';
    disp_txt_[2] = '-';
    disp_txt_[3] = 0;
    show_text = true;
  } else if (state_ >= kStateBase &&
             state_ < kStateBase + 16 * ir::RETX_QUEUE_SIZE) {
    int slot = (state_ - kStateBase) / 16;
    int sub_state = (state_ - kStateBase) % 16;

    uint8_t status = ir::irController.GetSlotStatusForDebug(slot);

    if (sub_state == 0) {
      if (status == ir::kRetransmitStatusWaitAck) {
        // Found a packet waiting for ACK, send it via XBoard.
        // The data is in queued_packets_[slot].data, and size is in .size
        // This requires friend access.
        service::xboard::g_xboard_logic.SendIRPacket(
            &(ir::irController.queued_packets_[slot].data[0]),
            ir::irController.queued_packets_[slot].size);
        tx_cnt_++;
        show_cycles_ = std::max(show_cycles_, kIrxbShowCyclesExtra);
        state_++;
      } else {
        // This slot is not waiting for an ACK, move to the next slot.
        state_ = kStateBase + (slot + 1) * 16;
      }
    } else if (sub_state < 5) {
      // Wait for 5 cycles (500ms)
      state_++;
    } else {  // sub_state is 5+
      // Done waiting, move to the next slot.
      state_ = kStateBase + (slot + 1) * 16;
    }
    set_txrx_text = true;
    show_text = true;
  } else if (state_ == kStateTxEnd) {
    // End of tx state, we'll check the cycles left/time out later.
    set_txrx_text = true;
    show_text = true;
  } else {
    // All slots checked, or invalid state, restart loop.
    state_ = kStateInit;
  }

  TamaRoutine();
  ScoreRoutine();

  if (set_txrx_text) {
    tx_cnt_ = std::min(tx_cnt_, 15);
    rx_cnt_ = std::min(rx_cnt_, 15);
    if (hitcon::service::xboard::g_xboard_service.IsTxBusy()) {
      disp_txt_[0] = '\\';
    } else {
      disp_txt_[0] = '/';
    }
    disp_txt_[1] = hitcon::uint_to_chr_hex_nibble(tx_cnt_);
    disp_txt_[2] = hitcon::uint_to_chr_hex_nibble(rx_cnt_);
    disp_txt_[3] = 0;
  }
  if (show_text) {
    display_set_mode_text(disp_txt_);
  }
  if (show_cycles_ > 0) {
    show_cycles_--;
    if (show_cycles_ == 0) {
      state_ = kStateMenu;
      connect_basestn_menu.NotifyIrXbFinished();
    }
  }
  return true;
}

void IrxbBridge::TamaRoutine() {
  // skip to "done" directly if we already received a restore packet.
  if (tama_app.PollRestorePacket()) {
    tama_state_ = TamaState::kTamaStateDone;
  }
  if (tama_state_ == TamaState::kTamaStateInit) {
    // Prepare a "save tama" packet.
    tama_data_.ttl = 0;
    tama_data_.type = packet_type::kSavePet;
    tama_app.SaveToBuffer(tama_data_.opaq.save_pet.pet_data);
    bool ret =
        g_game_controller.SetBufferToUsername(tama_data_.opaq.save_pet.user);
    if (ret) tama_state_ = TamaState::kTamaStateWaitSignStart;

  } else if (tama_state_ == TamaState::kTamaStateWaitSignStart) {
    sizeof(tama_data_.opaq.save_pet) - ECC_SIGNATURE_SIZE;
    bool ret = g_ec_logic.StartSign(
        reinterpret_cast<uint8_t *>(&tama_data_.opaq.save_pet),
        sizeof(tama_data_.opaq.save_pet) - ECC_SIGNATURE_SIZE,
        (callback_t)&IrxbBridge::OnTamaSignDone, this);
    if (ret) tama_state_ = TamaState::kTamastateWaitSignDone;
  } else if (tama_state_ == TamaState::kTamastateWaitSignDone) {
    // nop
  } else if (tama_state_ == TamaState::kTamaStateWaitSend) {
    bool ret = service::xboard::g_xboard_logic.SendIRPacket(
        reinterpret_cast<uint8_t *>(&tama_data_),
        sizeof(tama_data_.opaq.save_pet) + hitcon::ir::IR_DATA_HEADER_SIZE);
    if (ret) tama_state_ = TamaState::kTamaStateWaitRestore;
  } else if (tama_state_ == TamaState::kTamaStateWaitRestore) {
    // nop
  } else if (tama_state_ == TamaState::kTamaStateDone) {
    // nop
  } else {
    // Invalid state, reset to init.
    tama_state_ = TamaState::kTamaStateInit;
  }
}

void IrxbBridge::OnTamaSignDone(hitcon::ecc::Signature *signature) {
  signature->toBuffer(tama_data_.opaq.save_pet.sig);
  tama_state_ = TamaState::kTamaStateWaitSend;
}

void IrxbBridge::ScoreRoutine() {
  // If we already received a score packet, skip to "done".
  if (show_name_app.PollSetScorePacket())
    score_state_ = ScoreState::kScoreStateDone;

  if (score_state_ == ScoreState::kScoreStateInit) {
    score_data_.ttl = 0;
    score_data_.type = packet_type::kRequestScore;
    bool ret = g_game_controller.SetBufferToUsername(
        score_data_.opaq.request_score.user);
    if (ret) score_state_ = ScoreState::kScoreStateWaitSendGetScore;
  } else if (score_state_ == ScoreState::kScoreStateWaitSendGetScore) {
    bool ret = hitcon::service::xboard::g_xboard_logic.SendIRPacket(
        reinterpret_cast<uint8_t *>(&score_data_),
        sizeof(score_data_.opaq.request_score) +
            hitcon::ir::IR_DATA_HEADER_SIZE);
    if (ret) score_state_ = ScoreState::kScoreStateWaitSetScore;
  } else if (score_state_ == ScoreState::kScoreStateWaitSetScore) {
    // nop
  } else if (score_state_ == ScoreState::kScoreStateDone) {
    // nop
  } else {
    // Invalid state, reset to init.
    score_state_ = ScoreState::kScoreStateInit;
  }
}

void IrxbBridge::OnPacketReceived(void *arg) {
  service::xboard::PacketCallbackArg *packet_arg =
      reinterpret_cast<service::xboard::PacketCallbackArg *>(arg);

  rx_cnt_++;
  show_cycles_ = std::max(show_cycles_, kIrxbShowCyclesExtra);

  hitcon::service::sched::my_assert(packet_arg->len <=
                                    hitcon::ir::MAX_PACKET_PAYLOAD_BYTES);

  hitcon::ir::IrPacket pkt;
  pkt.data_[0] = 0;
  memcpy(&pkt.data_[1], packet_arg->data, packet_arg->len);
  pkt.size_ = packet_arg->len;

  ir::irController.OnPacketReceived(&pkt);
}

}  // namespace hitcon
