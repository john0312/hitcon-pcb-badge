#include <App/ShowNameApp.h>
#include <App/UsbMenuApp.h>
#include <Logic/BadgeController.h>

#include "main.h"

// handle USB_DET has power supply
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == USB_DET_Pin) {
    if (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == GPIO_PIN_SET) {
      hitcon::badge_controller.OnUsbPlugIn();
    } else {
      if (hitcon::badge_controller.GetCurrentApp() == &hitcon::usb::usb_menu) {
        hitcon::badge_controller.change_app(&hitcon::show_name_app);
      }
    }
  }
}

namespace hitcon {
namespace usb {
void SendBadgeID() { display_set_mode_scroll_text("Tuzki"); }
}  // namespace usb
}  // namespace hitcon
