#ifndef HITCON_APP_IRFORCERETXAPP_H_
#define HITCON_APP_IRFORCERETXAPP_H_

#include <App/app.h>
#include <Service/Sched/DelayedTask.h>

namespace hitcon {

class IrForceRetxApp : public App {
 public:
  IrForceRetxApp();
  virtual ~IrForceRetxApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  void RoutineTask();

  hitcon::service::sched::DelayedTask routine_task_;
  int state_;
};

extern IrForceRetxApp g_ir_force_retx_app;

}  // namespace hitcon

#endif  // HITCON_APP_IRFORCERETXAPP_H_
