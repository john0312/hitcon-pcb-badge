#ifndef SEND_DATA_APP_H_
#define SEND_DATA_APP_H_

#include <App/app.h>
#include <Logic/Display/display.h>
#include <Logic/XBoardGameController.h>
#include <Service/Sched/SysTimer.h>

namespace hitcon {

using hitcon::game::gameLogic;
using hitcon::service::sched::scheduler;
using hitcon::service::sched::SysTimer;

class SendDataApp : public App {
 public:
  SendDataApp();
  virtual ~SendDataApp() = default;

  void Init();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  bool running_;
  bool in_queue_;

  void Routine(void* args);
  void RoutineInternal();

  hitcon::service::sched::DelayedTask routine_task_delayed;
};

extern SendDataApp g_send_data_app;

}  // namespace hitcon

#endif  // SEND_DATA_APP_H_
