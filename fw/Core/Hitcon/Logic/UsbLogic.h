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

constexpr unsigned REPORT_LEN = USBD_CUSTOMHID_OUTREPORT_BUF_SIZE;
constexpr unsigned KEYBOARD_REPORT_ID = 1;
constexpr unsigned CUSTOM_REPORT_ID = 2;

constexpr unsigned SCRIPT_FLASH_INDEX = 0;
// USB_STATE_START_WRITE + script length (2 Bytes) + crc32 (4 Bytes)
constexpr unsigned SCRIPT_BEGIN_ADDR =
    FLASH_END_ADDR - FLASH_PAGE_COUNT * MY_FLASH_PAGE_SIZE + 1 + 7;
constexpr unsigned SCRIPT_LEN_ADDR = SCRIPT_BEGIN_ADDR - 6;
constexpr unsigned CRC32_ADDR = SCRIPT_BEGIN_ADDR - 2;

// storage size for script
constexpr uint16_t MAX_SCRIPT_LEN = MY_FLASH_PAGE_SIZE - 7;
constexpr char EMPTY_SCRIPT_MSG[] = "No script";
constexpr char CRC_FAIL_MSG[] = "Checksum fail";

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

struct WriteMemPacket {
  union {
    uint8_t u8[8];
    struct {
      uint32_t addr;
      uint32_t content;
    } s;
  };
};
struct ReadMemPacket {
  union {
    uint8_t u8[4];
    uint32_t addr;
  };
};

enum usb_state_t {
  USB_STATE_IDLE = 0,
  USB_STATE_SET_NAME = 1,
  USB_STATE_ERASE,
  USB_STATE_START_WRITE,
  USB_STATE_WRITE_MEM,
  USB_STATE_READ_MEM,
  USB_STATE_WRITING,
  USB_STATE_WAITING,     // waiting flash service done program
  USB_STATE_WAIT_ERASE,  // waiting erase done
  USB_STATE_RETRY,       // if SendKeyCode failed, retry
};

enum {  // script code definition
  CODE_DELAY = 0xFF,
  CODE_MODIFIER = 0xFE,
  CODE_RELEASE = 0x00
};

// MCU send this when action is done
// e.g. program partial done, set name, r/w memory
constexpr uint8_t CODE_ACTION_DONE = 0xFF;

enum mem_type_t {  // definiton for memory read/write type
  MEM_BYTE = 1,
  MEM_HALFWORD,
  MEM_WORD
};

class UsbLogic {
 private:
  // run routine task every 20 ms
  static constexpr unsigned DELAY_INTERVAL = 20;
  static constexpr unsigned WAIT_INTERVAL = 10;

  // Report _report;
  usb_state_t _state;
  int16_t _script_index;
  uint16_t _script_len;
  uint16_t _program_index;
  bool _new_data;
  bool _script_crc_flag;
  // store script data used in writeRoutine
  uint8_t _script_temp[REPORT_LEN - 1];
  Report _report;

  hitcon::service::sched::PeriodicTask _routine_task;
  hitcon::service::sched::PeriodicTask _write_routine_task;
  void Routine(void* unused);
  void WriteRoutine(void* unused);
  callback_t _on_finish_cb;
  void* _on_finish_arg1;
  callback_t _on_err_cb;
  void* _on_err_arg1;

  // Send a keyboard report
  void SendKeyCode(uint8_t keycode, uint8_t modifier);

  // The data should be REPORT_LEN bytes long.
  void SendCustomReport(uint8_t* data);

 public:
  UsbLogic();
  void OnDataRecv(uint8_t* data);
  void RunScript(callback_t cb, void* arg1, callback_t err_cb, void* arg2,
                 bool check_crc);
  void StopScript();
  void Init();
};

extern UsbLogic g_usb_logic;

}  // namespace usb
}  // namespace hitcon

#endif
