#ifndef USB_SERVICE_H_
#define USB_SERVICE_H_

#include <Logic/NvStorage.h>
#include <Service/FlashService.h>
#include <Service/Sched/PeriodicTask.h>
#include <usbd_conf.h>
extern "C" {
void UsbServiceOnDataReceived(uint8_t* data);
}

namespace hitcon {
// TODO: change to logic
constexpr unsigned RECV_BUF_LEN = 8;
// first 2 bytes is script length
constexpr unsigned SCRIPT_BEGIN_ADDR =
    (FLASH_END_ADDR - MY_FLASH_PAGE_SIZE + 1) + 2;

// TODO: partial program
// TODO: change to last page from flash service
constexpr unsigned NAME_LEN = 16;

struct {
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keycode[6];
} keyboard_report, empty_report;

enum usb_state_t {
  USB_STATE_HEADER = 0,
  USB_STATE_SET_NAME = 1,
  USB_STATE_CLEAR,
  USB_STATE_START_WRITE,
  USB_STATE_STOP_WRITE,
};

enum {  // script code definition
  CODE_DELAY = 0xFF,
  CODE_MODIFIER = 0xFE,
  CODE_FINISH = 0xFD,
  CODE_RELEASE = 0x00
};

class UsbLogic {
 private:
  // run routine task every 20 ms
  static constexpr unsigned DELAY_INTERVAL = 20;

  usb_state_t _state;
  int32_t _index;
  unsigned char _name[NAME_LEN];  // TODO: remove this
  bool flag;
  hitcon::service::sched::PeriodicTask _routine_task;
  void Routine(void* unused);

 public:
  UsbLogic();
  void OnDataRecv(uint8_t* data);
  void RunScript();  // TODO: check connection status
  void Init();
};

extern UsbLogic g_usb_logic;

}  // namespace hitcon

#endif
