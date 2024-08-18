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
namespace usb {
constexpr unsigned RECV_BUF_LEN = 8;
// first 2 bytes is script length
constexpr unsigned SCRIPT_BEGIN_ADDR =
    FLASH_END_ADDR - FLASH_PAGE_COUNT * MY_FLASH_PAGE_SIZE + 1 + 3;
constexpr unsigned SCRIPT_FLASH_INDEX = 0;

void RunScriptWrapper();

struct {
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keycode[6];
} keyboard_report, empty_report;

enum usb_state_t {
  USB_STATE_HEADER = 0,  // idle
  USB_STATE_SET_NAME = 1,
  USB_STATE_ERASE,
  USB_STATE_START_WRITE,
  USB_STATE_WRITE_MEM,
  USB_STATE_READ_MEM,
  USB_STATE_WRITING,
  USB_STATE_WAITING  // waiting for flash service done
};

enum {  // script code definition
  CODE_DELAY = 0xFF,
  CODE_MODIFIER = 0xFE,
  CODE_RELEASE = 0x00
};

enum {  // definiton for memory read/write type
  MEM_BYTE = 1,
  MEM_HALFWORD,
  MEM_WORD
};

class UsbLogic {
 private:
  // run routine task every 20 ms
  static constexpr unsigned DELAY_INTERVAL = 20;
  static constexpr unsigned WAIT_INTERVAL = 10;

  usb_state_t _state;
  int32_t _index;
  uint16_t _script_len;
  bool flag;
  bool _new_data;
  struct WriteMemPacket {
    union {
      uint8_t u8[8];
      struct {
        uint32_t addr;
        uint32_t content;
      } s;
    };
  } _write_mem_packet;
  struct ReadMemPacket {
    union {
      uint8_t u8[4];
      uint32_t addr;
    };
  } _read_mem_packet;

  hitcon::service::sched::PeriodicTask _routine_task;
  hitcon::service::sched::PeriodicTask _write_routine_task;
  void Routine(void* unused);
  void WriteRoutine(void* unused);
  uint8_t _temp[RECV_BUF_LEN];
  callback_t _on_finish_cb;
  void* _on_finish_arg1;

 public:
  hitcon::service::sched::Task on_recv_task;
  UsbLogic();
  void OnDataRecv(void* arg);
  // TODO: check connection status
  void RunScript(callback_t _on_finish_cb, void* on_finish_arg1);
  void StopScript();
  void Init();
};

extern UsbLogic g_usb_logic;

}  // namespace usb
}  // namespace hitcon

#endif
