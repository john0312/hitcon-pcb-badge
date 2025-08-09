#include <App/ShowIdApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Service/Sched/SysTimer.h>
#include <Service/UsbService.h>

using namespace hitcon::service::sched;
using namespace hitcon::usb;

namespace hitcon {

ShowIdApp show_id_app;

void ShowIdApp::Init() { scheduler.Queue(&_type_id_task, nullptr); }

void ShowIdApp::OnEntry() {
  GetId(nullptr);
  if (_id_str[0] == 0)
    display_set_mode_scroll_text("Loading...");
  else
    display_set_mode_scroll_text(_id_str);
}

void ShowIdApp::OnExit() {}

void ShowIdApp::GetId(void* unused) {
  const uint8_t* data = g_game_controller.GetUsername();
  if (data == nullptr) {
    _get_id_task.SetWakeTime(SysTimer::GetTime() + 500);
    scheduler.Queue(&_get_id_task, nullptr);
  } else {
    for (uint8_t i = 0; i < ir::IR_USERNAME_LEN; i++) {
      _id_str[3 * i] = "0123456789ABCDEF"[data[i] >> 4];
      _id_str[3 * i + 1] = "0123456789ABCDEF"[data[i] & 0xF];
      _id_str[3 * i + 2] = ' ';
    }
    _id_str[ir::IR_USERNAME_LEN * 3 - 1] = 0;
  }
}

void ShowIdApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
      badge_controller.BackToMenu(this);
      break;
    case BUTTON_OK:
      // send badge id keycode
      if (g_usb_service.IsConnected()) {
        g_usb_service.SendKeyCode(0, 0);  // send release
        scheduler.EnablePeriodic(&_type_id_task);
        _routine_count = 0;
      }
      break;
    default:
      break;
  }
}

void ShowIdApp::TypeIdRoutine(void* unused) {
  static bool flag = false;
  if (g_usb_service.IsBusy()) return;
  // TODO: move send release to UsbService
  if (flag) {
    g_usb_service.SendKeyCode(0, 0);
    flag = false;
  } else {
    if (_routine_count == sizeof(_id_str) - 1) {
      scheduler.DisablePeriodic(&_type_id_task);
      return;
    }
    std::pair<uint8_t, uint8_t> key =
        UsbService::GetKeyCode(_id_str[_routine_count]);
    g_usb_service.SendKeyCode(key.first, key.second);
    _routine_count++;
    flag = true;
  }
}

}  // namespace hitcon
