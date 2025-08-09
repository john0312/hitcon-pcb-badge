#include <App/DebugApp.h>
#include <Logic/Display/display.h>
#include <Logic/ImuLogic.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/Sched/Task.h>
#include <Util/uint_to_str.h>

using namespace hitcon::service::sched;

namespace hitcon {

namespace {
void ByteToHex(uint8_t value, char* out) {
  out[0] = hitcon::uint_to_chr_hex_nibble(value >> 4);
  out[1] = hitcon::uint_to_chr_hex_nibble(value);
}
}  // namespace

DebugAccelApp g_debug_accel_app;
IrRetxDebugApp g_ir_retx_debug_app;
DebugApp g_debug_app;

DebugAccelApp::DebugAccelApp()
    : main_task_(850, (task_callback_t)&DebugAccelApp::MainTaskFn, this, 800),
      main_task_scheduled_(false) {}

void DebugAccelApp::OnEntry() {
  running_ = true;
  EnsureQueued();
}
void DebugAccelApp::OnExit() { running_ = false; }
void DebugAccelApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_BACK:
      badge_controller.BackToMenu(this);
      break;
    default:
      break;
  }
}

void DebugAccelApp::EnsureQueued() {
  if (!main_task_scheduled_ && running_) {
    main_task_.SetWakeTime(SysTimer::GetTime() + 800);
    scheduler.Queue(&main_task_, nullptr);
    main_task_scheduled_ = true;
  }
}

void DebugAccelApp::MainTaskFn(void* unused) {
  main_task_scheduled_ = false;
  if (!running_) {
    return;
  }
  EnsureQueued();

  int val = g_imu_logic.GetStep();
  val = val & (0x0FFF);
  constexpr char hex_map[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  disp_buff_[0] = hex_map[(val & 0xF0) >> 4];
  disp_buff_[1] = hex_map[val & 0xF];
  disp_buff_[2] = 0;
  display_set_mode_text(disp_buff_);
}

IrRetxDebugApp::IrRetxDebugApp()
    : MenuApp(nullptr, 0) {}  // We'll set actual entries in OnEntry

void IrRetxDebugApp::OnEntry() {
  // Build packet entries - iterate through all slots and collect active ones
  int menu_index = 1;
  for (int slot = 0; slot < 4 && menu_index < MAX_MENU_ENTRIES; slot++) {
    uint8_t status = hitcon::ir::irController.GetSlotStatusForDebug(slot);

    // Show any slot that is not unused (same criteria as
    // GetRetxSlotCountForDebug)
    if (status != hitcon::ir::kRetransmitStatusSlotUnused) {
      char* line = menu_texts_[menu_index];

      // Format: "TT:SS-RR#TTTT" where TT=packet type, SS=Status, RR=retry
      // count, TTTT=time to retry
      uint8_t packet_type =
          hitcon::ir::irController.GetSlotPacketTypeForDebug(slot);
      ByteToHex(packet_type, line);
      line[2] = ':';

      line[3] = uint_to_chr_hex_nibble(status >> 4);
      line[4] = '-';

      uint8_t retry_count =
          hitcon::ir::irController.GetSlotRetryCountForDebug(slot);
      line[5] = uint_to_chr_hex_nibble(retry_count & 0x0F);
      line[6] = '#';

      uint16_t time_to_retry =
          hitcon::ir::irController.GetSlotTimeToRetryForDebug(slot);
      ByteToHex(static_cast<uint8_t>(time_to_retry >> 8), &line[7]);
      ByteToHex(static_cast<uint8_t>(time_to_retry & 0xFF), &line[9]);
      line[11] = '\0';

      menu_index++;
    }
  }

  // Build header "PKT:XX" showing total active slots
  menu_used_ = menu_index;
  uint8_t pkt_cnt = menu_index - 1;

  char* header = menu_texts_[0];
  header[0] = 'P';
  header[1] = 'K';
  header[2] = 'T';
  header[3] = ':';
  header[4] = uint_to_chr_hex_nibble(pkt_cnt);
  header[5] = '\0';

  // Build menu entries using class storage
  for (int i = 0; i < menu_index; i++) {
    menu_entries_[i].name = menu_texts_[i];
    menu_entries_[i].app = nullptr;
    menu_entries_[i].func = nullptr;
  }

  // Update menu pointers and size
  AdjustMenuPointer(menu_entries_, menu_index, true);

  // Call parent OnEntry to activate the menu
  MenuApp::OnEntry();
}

void IrRetxDebugApp::OnExit() { MenuApp::OnExit(); }

}  // namespace hitcon
