#ifndef SERVICE_USB_SERVICE_H_
#define SERVICE_USB_SERVICE_H_

#include <Service/Sched/Task.h>
#include <Util/callback.h>

#include <cstdint>
#include <utility>

#include "usbd_conf.h"

extern "C" {
void UsbServiceOnDataReceived(uint8_t* data);
}

namespace hitcon {
namespace usb {

constexpr unsigned REPORT_LEN = USBD_CUSTOMHID_OUTREPORT_BUF_SIZE;
constexpr unsigned KEYBOARD_REPORT_ID = 1;
constexpr unsigned CUSTOM_REPORT_ID = 2;

struct Report {
  uint8_t report_id;
  union {
    uint8_t u8[REPORT_LEN - 1];
    struct {
      uint8_t modifier;
      uint8_t reserved;
      uint8_t keycode[6];
    } keyboard_report;
  };
};

class UsbService {
 public:
  UsbService()
      : _retry_task(809,
                    (service::sched::task_callback_t)&UsbService::RetryHandler,
                    (void*)this, 20) {}

  // set callback when report id 2 is received
  void SetOnReportReceived();

  void SetOnDataRecv(callback_t cb, void* thisptr) {
    on_recv_cb = {cb, thisptr};
  }

  // Handle USB connection events.
  void SetOnUsbPlugIn(callback_t cb, void* thisptr) {
    on_plug_in_cb = {cb, thisptr};
  }

  // Handle USB disconnection events.
  void SetOnUsbPlugOut(callback_t cb, void* thisptr) {
    on_plug_out_cb = {cb, thisptr};
  }

  // Send a keyboard report
  void SendKeyCode(uint8_t keycode, uint8_t modifier);

  // The data should be REPORT_LEN bytes long.
  void SendCustomReport(uint8_t* data);
  bool IsBusy() { return _retrying; }

  // Handle USB_DET rising/falling external interrupt
  void InterruptHandler(GPIO_PinState state);

  // Handle report received from host
  void OnRecvReport(uint8_t* data);

 private:
  service::sched::DelayedTask _retry_task;
  bool _retrying = false;
  Report _report;

  std::pair<callback_t, void*> on_recv_cb = {nullptr, nullptr};
  std::pair<callback_t, void*> on_plug_in_cb = {nullptr, nullptr};
  std::pair<callback_t, void*> on_plug_out_cb = {nullptr, nullptr};

  void RetryHandler(void* unused);
};

extern UsbService g_usb_service;
}  // namespace usb
}  // namespace hitcon

#endif
