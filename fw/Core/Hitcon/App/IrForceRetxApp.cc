#include <App/IrForceRetxApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>

namespace hitcon {

IrForceRetxApp g_ir_force_retx_app;

IrForceRetxApp::IrForceRetxApp()
    : routine_task_(950, (callback_t)&IrForceRetxApp::RoutineTask, this, 500),
      state_(0) {}

void IrForceRetxApp::OnEntry() {
  state_ = 1;
  hitcon::service::sched::scheduler.Queue(&routine_task_, nullptr);
}

void IrForceRetxApp::OnExit() { state_ = 0; }

void IrForceRetxApp::OnButton(button_t button) {
  if (button == BUTTON_BACK) {
    badge_controller.BackToMenu(this);
  }
}

void IrForceRetxApp::RoutineTask() {
  if (state_ == 0) return;

  if (state_ == 1) {
    display_set_mode_text("E0");
    state_ = 128;
  } else if (state_ >= 128 && state_ < 128 + 16 * 16) {
    int slot = (state_ - 128) / 16;
    if (slot >= ir::RETX_QUEUE_SIZE) {
      state_ = 128 + 16 * 16;
    } else {
      int sub_state = (state_ - 128) % 16;
      uint8_t status = ir::irController.GetSlotStatusForDebug(slot);

      if (sub_state == 0) {
        if (status == ir::kRetransmitStatusWaitAck) {
          ir::irController.ForceRetransmitForDebug(slot);
          char buf[4];
          buf[0] = 'S';
          buf[1] = uint_to_chr_hex_nibble(slot);
          buf[2] = 0;
          display_set_mode_text(buf);
          state_++;
        } else {
          state_ = 128 + (slot + 1) * 16;
        }
      } else if (sub_state < 14) {
        if (status == ir::kRetransmitStatusWaitAck ||
            status == ir::kRetransmitStatusSlotUnused) {
          state_ = 128 + (slot + 1) * 16;
        } else {
          state_++;
        }
      } else {  // sub_state is 14 or 15
        if (status == ir::kRetransmitStatusWaitAck ||
            status == ir::kRetransmitStatusSlotUnused) {
          state_ = 128 + (slot + 1) * 16;
        } else {
          // wait
        }
      }
    }
  } else if (state_ == 128 + 16 * 16) {
    display_set_mode_text("E1");
    state_++;
  } else if (state_ > 128 + 16 * 16 && state_ < 128 + 16 * 16 + 5) {
    state_++;
  } else if (state_ == 128 + 16 * 16 + 5) {
    badge_controller.BackToMenu(this);
  }

  if (state_ != 0) {
    hitcon::service::sched::scheduler.Queue(&routine_task_, nullptr);
  }
}

}  // namespace hitcon
