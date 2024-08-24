#ifndef BAD_USB_APP_H
#define BAD_USB_APP_H

#include <App/app.h>

namespace hitcon {
namespace usb {

class BadUsbApp : public App {
 private:
  bool _wait;
  void OnScriptFinished(void* unused);
  void OnScriptError(void* msg);

 public:
  BadUsbApp();
  virtual ~BadUsbApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern BadUsbApp bad_usb_app;
}  // namespace usb
}  // namespace hitcon

#endif
