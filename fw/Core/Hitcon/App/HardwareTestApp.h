#ifndef HARDWARE_TEST_APP
#define HARDWARE_TEST_APP

#include "app.h"
#include <Service/Sched/PeriodicTask.h>

using namespace hitcon::service::sched;
namespace hitcon {
enum TEST_APP_STATE{
  TS_DISPLAY_TEST = 1,
  TS_BTN_BRIGHTNESS,
  TS_BTN_BACK,
  TS_BTN_UP,
  TS_BTN_MODE,
  TS_BTN_LEFT,
  TS_BTN_OK,
  TS_BTN_RIGHT,
  TS_BTN_DOWN,
//  TS_BTN_LONG_BRIGHTNESS,
//  TS_BTN_LONG_BACK,
//  TS_BTN_LONG_UP,
//  TS_BTN_LONG_MODE,
//  TS_BTN_LONG_LEFT,
//  TS_BTN_LONG_OK,
//  TS_BTN_LONG_RIGHT,
//  TS_BTN_LONG_DOWN,
  TS_IR_SEND,
  TS_IR_RECV,
  TS_XBOARD_SEND,
  TS_XBOARD_RECV
};
class HardwareTestApp : public App {
public:
  HardwareTestApp();
  virtual ~HardwareTestApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void Init();
  void Routine(void* unused);
  PeriodicTask task;

private:
  uint32_t current_state;
  uint32_t next_state;
  uint32_t start_tick;
};

extern HardwareTestApp hardware_test_app;

} // namespace hitcon

#endif // HARDWARE_TEST_APP
