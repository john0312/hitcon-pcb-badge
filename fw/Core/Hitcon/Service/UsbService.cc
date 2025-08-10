#include <Logic/Display/display.h>
#include <Service/Sched/Checks.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>
#include <Service/UsbService.h>
#include <usb_device.h>
#include <usbd_custom_hid_if.h>

#include "main.h"

// handle USB_DET has power supply
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == USB_DET_Pin) {
    bool state =
        (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == GPIO_PIN_SET);
    scheduler.Queue(&hitcon::usb::g_usb_service.interrupt_handler_task,
                    reinterpret_cast<void*>(state));
  }
}

namespace hitcon {
namespace usb {

UsbService g_usb_service;

void UsbService::InterruptHandler(void* arg2) {
  bool state = static_cast<bool>(arg2);
  if (state) {
    if (on_plug_in_cb.first) {
      on_plug_in_cb.first(on_plug_in_cb.second, nullptr);
    }
    _connected = true;
  } else {
    if (on_plug_out_cb.first) {
      on_plug_out_cb.first(on_plug_out_cb.second, nullptr);
    }
    _connected = false;
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

std::pair<uint8_t, uint8_t> UsbService::GetKeyCode(char c) {
  // keycode, modifier
  std::pair<uint8_t, uint8_t> key = {0, 0};
  if ('0' <= c && c <= '9') {
    if (c == '0')
      key.first = KEYCODE_0;
    else
      key.first = KEYCODE_1 + c - '1';
  } else if ('a' <= c && c <= 'z') {
    key.first = KEYCODE_A + c - 'a';
  } else if ('A' <= c && c <= 'Z') {
    key.first = KEYCODE_A + c - 'A';
    key.second = MODIFIER_LSHIFT;
  } else if (c == ' ') {
    key.first = KEYCODE_SPACE;
  } else {
    service::sched::my_assert(false);
  }
  return key;
}

}  // namespace usb
}  // namespace hitcon

void UsbServiceOnDataReceived(uint8_t* data) {
  hitcon::usb::g_usb_service.OnRecvReport(data);
}
