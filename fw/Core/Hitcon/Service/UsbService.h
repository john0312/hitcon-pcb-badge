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

using namespace hitcon::service::sched;

namespace hitcon {
namespace usb {

constexpr unsigned REPORT_LEN = USBD_CUSTOMHID_OUTREPORT_BUF_SIZE;
constexpr unsigned KEYBOARD_REPORT_ID = 1;
constexpr unsigned CUSTOM_REPORT_ID = 2;

constexpr uint8_t KEYCODE_A = 0x04;
constexpr uint8_t KEYCODE_1 = 0x1E;
constexpr uint8_t KEYCODE_0 = 0x27;
constexpr uint8_t KEYCODE_SPACE = 0x2c;
constexpr uint8_t MODIFIER_LSHIFT = 0x02;

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
      : interrupt_handler_task(
            808, (task_callback_t)&UsbService::InterruptHandler, (void*)this),
        _retry_task(809, (task_callback_t)&UsbService::RetryHandler,
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
  bool IsConnected() { return _connected; }

  // Handle report received from host
  void OnRecvReport(uint8_t* data);

  // translate char (A-Za-z0-9) to keycode
  // return keycode (first) and modifier (second)
  static std::pair<uint8_t, uint8_t> GetKeyCode(char c);

  Task interrupt_handler_task;

 private:
  DelayedTask _retry_task;
  bool _retrying = false;
  bool _connected = false;
  Report _report;

  std::pair<callback_t, void*> on_recv_cb = {nullptr, nullptr};
  std::pair<callback_t, void*> on_plug_in_cb = {nullptr, nullptr};
  std::pair<callback_t, void*> on_plug_out_cb = {nullptr, nullptr};

  void RetryHandler(void* unused);

  // Handle USB_DET rising/falling external interrupt
  void InterruptHandler(void* arg2);
};

extern UsbService g_usb_service;
}  // namespace usb
}  // namespace hitcon

#endif
