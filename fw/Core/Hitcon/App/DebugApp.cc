#include <App/DebugApp.h>
#include <Logic/Display/display.h>
#include <Logic/ImuLogic.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>
#include <Util/uint_to_str.h>

using namespace hitcon::service::sched;

namespace hitcon {

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

DebugAccelApp g_debug_accel_app;
DebugApp g_debug_app;

}  // namespace hitcon
