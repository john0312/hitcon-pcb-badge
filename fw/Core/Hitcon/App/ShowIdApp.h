#ifndef SHOW_ID_APP_H
#define SHOW_ID_APP_H

#include <App/app.h>
#include <Logic/IrController.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {
class ShowIdApp : public App {
 public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  ShowIdApp()
      : _get_id_task(900, (service::sched::task_callback_t)&ShowIdApp::GetId,
                     this, 50000),
        _type_id_task(
            810, (service::sched::task_callback_t)&ShowIdApp::TypeIdRoutine,
            this, 20) {}
#pragma GCC diagnostic pop
  virtual ~ShowIdApp() = default;

  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  service::sched::DelayedTask _get_id_task;
  service::sched::PeriodicTask _type_id_task;
  // store id string, e.g. "78 82 AD 2F" 2 5 8
  char _id_str[ir::IR_USERNAME_LEN * 3] = {0};
  void GetId(void* unused);
  void TypeIdRoutine(void* unused);
  uint8_t _routine_count = 0;
};

extern ShowIdApp show_id_app;
}  // namespace hitcon
#endif
