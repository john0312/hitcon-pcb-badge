#include <App/MenuApp.h>
#include <Logic/BadgeController.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

#include "MenuApp.h"

namespace hitcon {

// =========== Accel Debug App ===========

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

// =========== IrRetx Debug App ===========

class IrRetxDebugApp : public MenuApp {
 public:
  static constexpr int MAX_MENU_ENTRIES = 5;  // 1 header + max 4 packets
  static constexpr int MENU_ENTRY_LEN = 16;

  IrRetxDebugApp();  // Updated constructor initialization
  virtual ~IrRetxDebugApp() = default;

  void OnEntry() override;
  void OnExit() override;

  void OnButtonMode() override {};
  void OnButtonBack() override { badge_controller.BackToMenu(this); }
  void OnButtonLongBack() override { badge_controller.BackToMenu(this); }

 private:
  char menu_texts_[MAX_MENU_ENTRIES][MENU_ENTRY_LEN];
  menu_entry_t menu_entries_[MAX_MENU_ENTRIES];
  uint8_t menu_used_;
};

extern IrRetxDebugApp g_ir_retx_debug_app;

// =========== Main Debug App ===========

constexpr menu_entry_t debug_menu_entries[] = {
    {"Accel", &g_debug_accel_app, nullptr},
    {"IR Retx", &g_ir_retx_debug_app, nullptr}};

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
