#include <App/MenuApp.h>
#include <Logic/BadgeController.h>
#include <Service/Sched/Scheduler.h>

#include "MenuApp.h"

namespace hitcon {

class DebugAccelApp : public App {
 public:
  DebugAccelApp();
  virtual ~DebugAccelApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  hitcon::service::sched::DelayedTask main_task_;
  bool main_task_scheduled_;
  bool running_;
  char disp_buff_[10];

  void MainTaskFn(void *unused);
  void EnsureQueued();
};

extern DebugAccelApp g_debug_accel_app;

constexpr menu_entry_t debug_menu_entries[] = {
    {"Accel", &g_debug_accel_app, nullptr}};

constexpr size_t debug_menu_entries_len =
    sizeof(debug_menu_entries) / sizeof(debug_menu_entries[0]);

class DebugApp : public MenuApp {
 public:
  DebugApp() : MenuApp(debug_menu_entries, debug_menu_entries_len) {}

  void OnButtonMode() override {}
  void OnButtonBack() override { badge_controller.BackToMenu(this); }
  void OnButtonLongBack() override { badge_controller.BackToMenu(this); }
};

extern DebugApp g_debug_app;

}  // namespace hitcon
