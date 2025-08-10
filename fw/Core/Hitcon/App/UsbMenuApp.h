#ifndef USB_MENU_APP_H
#define USB_MENU_APP_H

#include <App/BadUsbApp.h>
#include <App/MainMenuApp.h>
#include <App/MenuApp.h>
#include <App/ShowIdApp.h>
#include <Logic/BadgeController.h>

namespace hitcon {
namespace usb {

extern void SendBadgeID();

constexpr menu_entry_t usb_menu_entries[] = {
    {"Show ID", &show_id_app, nullptr},
    {"BadUSB", &bad_usb_app, nullptr},
};

constexpr int usb_menu_entries_len =
    sizeof(usb_menu_entries) / sizeof(menu_entry_t);

class UsbMenuApp : public MenuApp {
 public:
  UsbMenuApp() : MenuApp(usb_menu_entries, usb_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override { badge_controller.change_app(&main_menu); }
};

extern UsbMenuApp usb_menu;
}  // namespace usb
}  // namespace hitcon

#endif
