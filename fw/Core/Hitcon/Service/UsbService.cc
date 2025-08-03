#include <Logic/Display/display.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/UsbService.h>
#include <usb_device.h>
#include <usbd_custom_hid_if.h>

#include "main.h"

// handle USB_DET has power supply
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == USB_DET_Pin) {
    hitcon::usb::g_usb_service.InterruptHandler(
        HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin));
  }
}

namespace hitcon {
namespace usb {
void SendBadgeID() { display_set_mode_scroll_text("Tuzki"); }

UsbService g_usb_service;

void UsbService::InterruptHandler(GPIO_PinState state) {
  if (state == GPIO_PIN_SET) {
    if (on_plug_in_cb.first) {
      on_plug_in_cb.first(on_plug_in_cb.second, nullptr);
    }
  } else {
    if (on_plug_out_cb.first) {
      on_plug_out_cb.first(on_plug_out_cb.second, nullptr);
    }
  }
}

void UsbService::OnRecvReport(uint8_t* data) {
  if (data[0] == CUSTOM_REPORT_ID) {
    if (on_recv_cb.first) {
      on_recv_cb.first(on_recv_cb.second, data);
    }
  }
}

void UsbService::SendKeyCode(uint8_t keycode, uint8_t modifier) {
  _report.report_id = KEYBOARD_REPORT_ID;
  memset(_report.u8, 0, 8);
  _report.keyboard_report.keycode[0] = keycode;
  _report.keyboard_report.modifier = modifier;
  auto ret = USBD_CUSTOM_HID_SendReport(
      &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);

  if (ret != USBD_OK) {
    _retry_task.SetWakeTime(10 + service::sched::SysTimer::GetTime());
    service::sched::scheduler.Queue(&_retry_task, nullptr);
    _retrying = true;
  } else
    _retrying = false;
}

// TODO: add retry
void UsbService::SendCustomReport(uint8_t* data) {
  _report.report_id = CUSTOM_REPORT_ID;
  memcpy(_report.u8, data, REPORT_LEN - 1);
  USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                             reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);
}

void UsbService::RetryHandler(void* unused) {
  auto ret = USBD_CUSTOM_HID_SendReport(
      &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);

  if (ret != USBD_OK) {
    _retry_task.SetWakeTime(10 + service::sched::SysTimer::GetTime());
    service::sched::scheduler.Queue(&_retry_task, nullptr);
    _retrying = true;
  } else
    _retrying = false;
}

}  // namespace usb
}  // namespace hitcon

void UsbServiceOnDataReceived(uint8_t* data) {
  hitcon::usb::g_usb_service.OnRecvReport(data);
}