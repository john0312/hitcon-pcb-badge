#include <App/ConnectMenuApp.h>
#include <Logic/Display/display.h>
#include <Logic/IrLogic.h>
#include <Logic/IrxbBridge.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Util/uint_to_str.h>

#include <algorithm>

using hitcon::service::sched::SysTimer;

namespace hitcon {

namespace {
constexpr unsigned kIrxbDelayTime = 100;
constexpr int kIrxbShowCyclesStart = 60;
constexpr int kIrxbShowCyclesExtra = 50;
constexpr unsigned kStateBase = 32;
constexpr unsigned kStateTxEnd = kStateBase + 16 * ir::RETX_QUEUE_SIZE;
constexpr unsigned kStateMenu = 2;
constexpr unsigned kStateInit = 1;
}  // namespace

IrxbBridge g_irxb_bridge;

IrxbBridge::IrxbBridge()
    : routine_task_(950, (callback_t)&IrxbBridge::RoutineTask, this,
                    kIrxbDelayTime),
      state_(0) {}

void IrxbBridge::Init() {
  service::xboard::g_xboard_logic.SetOnPacketArrive(
      (callback_t)&IrxbBridge::OnPacketReceived, this,
      service::xboard::RecvFnId::IR_TO_ATTENDEE);
}

void IrxbBridge::OnXBoardBasestnConnect() {
  state_ = 1;
  show_cycles_ = kIrxbShowCyclesStart;
  tx_cnt_ = rx_cnt_ = 0;

  routine_task_.SetWakeTime(SysTimer::GetTime() + kIrxbDelayTime);
  service::sched::scheduler.Queue(&routine_task_, nullptr);
}

void IrxbBridge::OnXBoardBasestnDisconnect() { state_ = 0; }

void IrxbBridge::RoutineTask() {
  bool ret = RoutineInternal();
  if (!ret) return;
  routine_task_.SetWakeTime(SysTimer::GetTime() + kIrxbDelayTime);
  service::sched::scheduler.Queue(&routine_task_, nullptr);
}

bool IrxbBridge::RoutineInternal() {
  if (state_ == 0) return false;

  if (state_ == kStateMenu) return true;

  bool show_text = false;
  bool set_txrx_text = false;
  if (state_ == kStateInit) {
    // Start of the loop
    state_ = kStateBase;
    disp_txt_[0] = 'B';
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

  if (set_txrx_text) {
    tx_cnt_ = std::min(tx_cnt_, 15);
    rx_cnt_ = std::min(rx_cnt_, 15);
    disp_txt_[0] = 'B';
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

void IrxbBridge::OnPacketReceived(void* arg) {
  service::xboard::PacketCallbackArg* packet_arg =
      reinterpret_cast<service::xboard::PacketCallbackArg*>(arg);

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
