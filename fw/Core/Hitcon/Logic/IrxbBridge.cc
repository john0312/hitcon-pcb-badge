#include <Logic/IrLogic.h>
#include <Logic/IrxbBridge.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <string.h>

using hitcon::service::sched::SysTimer;

namespace hitcon {

constexpr uint32_t kIrxbDelayTime = 100;

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
  routine_task_.SetWakeTime(SysTimer::GetTime() + kIrxbDelayTime);
  service::sched::scheduler.Queue(&routine_task_, nullptr);
}

void IrxbBridge::OnXBoardBasestnDisconnect() { state_ = 0; }

void IrxbBridge::RoutineTask() {
  if (state_ == 0) return;

  if (state_ == 1) {
    // Start of the loop
    state_ = 128;
  } else if (state_ >= 128 && state_ < 128 + 16 * ir::RETX_QUEUE_SIZE) {
    int slot = (state_ - 128) / 16;
    int sub_state = (state_ - 128) % 16;

    uint8_t status = ir::irController.GetSlotStatusForDebug(slot);

    if (sub_state == 0) {
      if (status == ir::kRetransmitStatusWaitAck) {
        // Found a packet waiting for ACK, send it via XBoard.
        // The data is in queued_packets_[slot].data, and size is in .size
        // This requires friend access.
        service::xboard::g_xboard_logic.SendIRPacket(
            &(ir::irController.queued_packets_[slot].data[0]),
            ir::irController.queued_packets_[slot].size);
        state_++;
      } else {
        // This slot is not waiting for an ACK, move to the next slot.
        state_ = 128 + (slot + 1) * 16;
      }
    } else if (sub_state < 2) {
      // Wait for 2 cycles (200ms)
      state_++;
    } else {  // sub_state is 2
      // Done waiting, move to the next slot.
      state_ = 128 + (slot + 1) * 16;
    }
  } else if (state_ == 128 + 16 * ir::RETX_QUEUE_SIZE) {
    // End state, let's leave it here.
  } else {
    // All slots checked, or invalid state, restart loop.
    state_ = 1;
  }

  if (state_ != 0) {
    routine_task_.SetWakeTime(SysTimer::GetTime() + kIrxbDelayTime);
    service::sched::scheduler.Queue(&routine_task_, nullptr);
  }
}

void IrxbBridge::OnPacketReceived(void* arg) {
  service::xboard::PacketCallbackArg* packet_arg =
      reinterpret_cast<service::xboard::PacketCallbackArg*>(arg);


  hitcon::service::sched::my_assert(packet_arg->len <= hitcon::ir::MAX_PACKET_PAYLOAD_BYTES);

  hitcon::ir::IrPacket pkt;
  pkt.data_[0] = 0;
  memcpy(&pkt.data_[1], packet_arg->data, packet_arg->len);
  pkt.size_ = packet_arg->len;

  ir::irController.OnPacketReceived(&pkt);
}

}  // namespace hitcon
