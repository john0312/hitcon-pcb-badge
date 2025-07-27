#ifndef USB_MENU_APP_H
#define USB_MENU_APP_H

#include <App/BadUsbApp.h>
#include <App/MenuApp.h>

namespace hitcon {
namespace usb {

extern void SendBadgeID();

constexpr menu_entry_t usb_menu_entries[] = {
    {"BadUSB", &bad_usb_app, nullptr},
    {"Badge ID", nullptr, &SendBadgeID},
};

constexpr int usb_menu_entries_len =
    sizeof(usb_menu_entries) / sizeof(menu_entry_t);

class UsbMenuApp : public MenuApp {
 public:
  UsbMenuApp() : MenuApp(usb_menu_entries, usb_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern UsbMenuApp usb_menu;
}  // namespace usb
}  // namespace hitcon

#endif
