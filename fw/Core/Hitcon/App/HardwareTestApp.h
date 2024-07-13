#ifndef HARDWARE_TEST_APP
#define HARDWARE_TEST_APP

#include <Service/Sched/PeriodicTask.h>

#include "app.h"

using namespace hitcon::service::sched;
namespace hitcon {
enum TEST_APP_STATE {  // TODO: add display: checkerboard and brightness
  TS_DISPLAY_SET_ALL = 1,
  TS_DISPLAY_RESET_ALL,
  TS_DISPLAY_CHECKER,
  TS_DISPLAY_BRIGHTNESS,
  TS_BTN_BRIGHTNESS,
  TS_BTN_BACK,
  TS_BTN_UP,
  TS_BTN_MODE,
  TS_BTN_LEFT,
  TS_BTN_OK,
  TS_BTN_RIGHT,
  TS_BTN_DOWN,
  TS_XBOARD,
  TS_IR,
  TS_PASS,
  TS_FAIL = 99,
};

constexpr int PERIOD = 1000;

class HardwareTestApp : public App {
 public:
  HardwareTestApp();
  virtual ~HardwareTestApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void Init();
  void Routine(void* unused);
  void CheckXBoard(void* arg);
  PeriodicTask task;

 private:
  uint32_t current_state;
  uint32_t next_state;
  uint32_t start_tick;
  uint8_t _count;

  uint32_t ir_ff_count;
  uint32_t ir_nonff_count;
  bool ir_found_3ff;
  bool ir_found_7ff;
  uint32_t ir_state;

  void CheckIrCount(uint32_t ff_count, uint32_t ir_nonff_count);
  void OnIrRx(void* arg1);
  void InitIRTest();
};

extern HardwareTestApp hardware_test_app;

}  // namespace hitcon

#endif  // HARDWARE_TEST_APP
