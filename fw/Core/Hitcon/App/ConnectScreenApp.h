#ifndef CONNECT_SCREEN_H
#define CONNECT_SCREEN_H

#include <Service/Sched/PeriodicTask.h>

#include "app.h"

namespace hitcon {
namespace app {
namespace connect_screen {
class ConnectScreen : public App {
 public:
  ConnectScreen();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void UpdateDisplay();

 private:
  hitcon::service::sched::PeriodicTask _routine_task;
};

extern ConnectScreen connect_screen;

}  // namespace connect_screen
}  // namespace app
}  // namespace hitcon

#endif  // CONNECT_SCREEN_H