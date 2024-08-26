#include <App/BadUsbApp.h>
#include <Logic/BadgeController.h>
#include <Logic/UsbLogic.h>

namespace hitcon {
namespace usb {
BadUsbApp bad_usb_app;

BadUsbApp::BadUsbApp() {}

void BadUsbApp::OnEntry() {
  _wait = false;
  g_usb_logic.RunScript((callback_t)&BadUsbApp::OnScriptFinished, this,
                        (callback_t)&BadUsbApp::OnScriptError, this, true);
}

void BadUsbApp::OnExit() { g_usb_logic.StopScript(); }

void BadUsbApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
      badge_controller.BackToMenu(this);
      break;
    case BUTTON_OK:
      if (_wait) {
        g_usb_logic.RunScript((callback_t)&BadUsbApp::OnScriptFinished, this,
                              (callback_t)&BadUsbApp::OnScriptError, this,
                              false);
      }
      break;
  }
}

void BadUsbApp::OnScriptFinished(void* unsed) {
  badge_controller.BackToMenu(this);
}

void BadUsbApp::OnScriptError(void* unused) {
  _wait = true;
  display_set_mode_scroll_text("checksum fail");
}

}  // namespace usb
}  // namespace hitcon
