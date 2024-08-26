#include <App/SendDataApp.h>
#include <Logic/XBoardGameController.h>
#include <Service/Sched/Scheduler.h>

using hitcon::xboard_game_controller::g_xboard_game_controller;

namespace hitcon {

namespace {

constexpr int kRoutineDelay = 500;

}  // namespace

SendDataApp g_send_data_app;

SendDataApp::SendDataApp()
    : routine_task_delayed(900, (callback_t)&SendDataApp::Routine, this, 0) {}

void SendDataApp::Init() {
  running_ = false;
  in_queue_ = false;
}

void SendDataApp::OnEntry() {
  g_xboard_game_controller.SendPartialData(25);
  running_ = true;
  if (!in_queue_) {
    in_queue_ = true;
    routine_task_delayed.SetWakeTime(SysTimer::GetTime());
    scheduler.Queue(&routine_task_delayed, nullptr);
  }
}

void SendDataApp::OnExit() { running_ = false; }

void SendDataApp::OnButton(button_t button) {
  // Unused.
}

void SendDataApp::Routine(void* arg) {
  in_queue_ = false;
  if (!running_) return;

  RoutineInternal();

  in_queue_ = true;
  routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kRoutineDelay);
  scheduler.Queue(&routine_task_delayed, nullptr);
}

void SendDataApp::RoutineInternal() {
  int left = g_xboard_game_controller.GetRemainingDataCount();
  if (left >= 99) left = 99;

  char temp[5];
  temp[0] = left / 10 + '0';
  temp[1] = left % 10 + '0';
  temp[2] = 0;
  display_set_mode_text(temp);
}

}  // namespace hitcon
