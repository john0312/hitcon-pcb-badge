#include "HardwareTestApp.h"
#include <Logic/Display/display.h>
#include <App/EditNameApp.h>
#include <Service/Sched/Scheduler.h>
#include <cstring>

using namespace hitcon::service::sched;
namespace hitcon {
HardwareTestApp hardware_test_app;
HardwareTestApp::HardwareTestApp() : task(30, (task_callback_t)&HardwareTestApp::Routine, (void*) this, 500) {

}

void HardwareTestApp::Init() {
  scheduler.Queue(&task, nullptr);
}

void HardwareTestApp::OnEntry() {
  next_state = TS_DISPLAY_TEST;
  start_tick = HAL_GetTick();
  scheduler.EnablePeriodic(&task);
}

void HardwareTestApp::OnExit() {
  scheduler.DisablePeriodic(&task);
}

void HardwareTestApp::OnButton(button_t button) {
  switch(current_state) {
    case TS_BTN_BRIGHTNESS:
      if(button == BUTTON_BRIGHTNESS) {
	next_state = TS_BTN_BACK;
      }
      break;
    case TS_BTN_BACK:
      if(button == BUTTON_BACK) {
	next_state = TS_BTN_UP;
      }
      break;
    case TS_BTN_UP:
      if(button == BUTTON_UP) {
	next_state = TS_BTN_MODE;
      }
      break;
    case TS_BTN_MODE:
      if(button == BUTTON_MODE) {
	next_state = TS_BTN_LEFT;
      }
      break;
    case TS_BTN_LEFT:
      if(button == BUTTON_LEFT) {
	next_state = TS_BTN_OK;
      }
      break;
    case TS_BTN_OK:
      if(button == BUTTON_OK) {
	next_state = TS_BTN_RIGHT;
      }
      break;
    case TS_BTN_RIGHT:
      if(button == BUTTON_RIGHT) {
	next_state = TS_BTN_DOWN;
      }
      break;
    case TS_BTN_DOWN:
      if(button == BUTTON_DOWN) {
	next_state = TS_IR_SEND;
      }
      break;
  }

}

void HardwareTestApp::Routine(void* unused) {
  if(current_state != next_state) {
    current_state = next_state;
    char temp[5];
    temp[0] = current_state / 10 + '0';
    temp[1] = current_state % 10 + '0';
    temp[2] = 0;
    display_set_mode_scroll_text(temp);
  }
  if(current_state == TS_DISPLAY_TEST) {
    uint8_t buf_fixed[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    display_set_mode_fixed(buf_fixed);
    if(HAL_GetTick() - start_tick > 2000)
      next_state = TS_BTN_BRIGHTNESS;
  }
}

} // namespace hitcon
