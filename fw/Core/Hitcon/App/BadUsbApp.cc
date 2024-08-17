#include <App/BadUsbApp.h>
#include <Logic/BadgeController.h>
#include <Logic/UsbLogic.h>

namespace hitcon {
namespace usb {
BadUsbApp bad_usb_app;

BadUsbApp::BadUsbApp() {}

void BadUsbApp::OnEntry() {
  g_usb_logic.RunScript((callback_t)&BadUsbApp::OnScriptFinished, this);
}

void BadUsbApp::OnExit() { g_usb_logic.StopScript(); }

void BadUsbApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
      badge_controller.OnAppEnd(this);
      break;
  }
}

void BadUsbApp::OnScriptFinished(void *unsed) {
  badge_controller.OnAppEnd(this);
}
}  // namespace usb
}  // namespace hitcon
