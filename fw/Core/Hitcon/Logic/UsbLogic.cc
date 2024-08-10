#include <App/ShowNameApp.h>
#include <Logic/NvStorage.h>
#include <Logic/UsbLogic.h>
#include <Service/Sched/Scheduler.h>
#include <main.h>
#include <usb_device.h>
#include <usbd_custom_hid_if.h>

using namespace hitcon::service::sched;
// TODO: check name length

namespace hitcon {
UsbLogic g_usb_logic;

UsbLogic::UsbLogic()
    : _routine_task(810, (task_callback_t)&UsbLogic::Routine, (void*)this,
                    DELAY_INTERVAL) {}

void UsbLogic::Init() {
  _state = USB_STATE_HEADER;
  _index = 0;
  scheduler.Queue(&_routine_task, nullptr);
}
/* TODO:
 * 1. check recv have buffer? or set
 * 2. get recv data length
 * 3. implement write script on flash
 * 4. test run script
 * 5. fix set_name
 */
void UsbLogic::OnDataRecv(uint8_t* data) {
  // check length?, call this function at outevent
  if (_state == USB_STATE_HEADER) {
    if (data[0]) {
      _state = static_cast<usb_state_t>(data[0]);
      _index = 0;
    }
  }
  nv_storage_content& content = g_nv_storage.GetCurrentStorage();

  FLASH_EraseInitTypeDef erase_struct = {
      .TypeErase = FLASH_TYPEERASE_PAGES,
      .PageAddress = SCRIPT_BEGIN_ADDR,
      .NbPages = MY_FLASH_PAGE_SIZE / FLASH_PAGE_SIZE,
  };
  uint32_t error;
  switch (_state) {
    case USB_STATE_SET_NAME:
      if (_index == NAME_LEN) {
        //        g_nv_storage.MarkDirty();
        show_name_app.SetName(reinterpret_cast<const char*>(_name));
        _state = USB_STATE_HEADER;
      }
      for (size_t i = (_index == 0); i < RECV_BUF_LEN; i++, _index++) {
        //        content.name[_index] = data[i];
        _name[_index] = data[i];
        // TODO: maybe need a temp buffer?
      }
      break;
    case USB_STATE_CLEAR:
      HAL_FLASH_Unlock();
      HAL_FLASHEx_Erase(&erase_struct, &error);
      HAL_FLASH_Lock();
      if (error != 0xFFFFFFFF) {
        // TODO: error erase flash
        return;
      }
      _state = USB_STATE_HEADER;
      break;
    case USB_STATE_START_WRITE:
      // TODO: use flash service to program page
      // TODO: add checksum

      break;
    case USB_STATE_STOP_WRITE:
      _state = USB_STATE_HEADER;
      break;
    default:
      break;
  }
}

void UsbLogic::RunScript() {
  _index = -1;
  keyboard_report = {0, 0, 0, 0, 0, 0, 0, 0};
  empty_report = {0, 0, 0, 0, 0, 0, 0, 0};
  scheduler.EnablePeriodic(&_routine_task);
  flag = false;
}

// run every 20ms
void UsbLogic::Routine(void* unused) {
  static uint8_t delay_count = 0;

  if (flag) {
    flag = false;

    if (keyboard_report.modifier == 0)
      USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                                 reinterpret_cast<uint8_t*>(&empty_report), 8);
    return;
  }
  if (delay_count != 0) {
    delay_count--;
    return;
  } else if (_index == *reinterpret_cast<uint16_t*>(SCRIPT_BEGIN_ADDR - 2)) {
    // TODO: check behavior when finish script
    scheduler.DisablePeriodic(&_routine_task);
    return;
  }
  if (_index == -1) {  // new script begin
    delay_count = 0;
  } else {
    uint8_t* addr = reinterpret_cast<uint8_t*>(SCRIPT_BEGIN_ADDR + _index);
    switch (*addr) {
      case CODE_DELAY:
        _index += 2;
        delay_count = *(addr + 1);
        return;
      case CODE_RELEASE:
        keyboard_report.modifier = 0;
        keyboard_report.keycode[0] = 0;
        break;
      case CODE_MODIFIER:
        _index++;
        keyboard_report.modifier = *(addr + 1);
        addr += 2;
      default:
        keyboard_report.keycode[0] = *addr;
        break;
    }
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                               reinterpret_cast<uint8_t*>(&keyboard_report), 8);
    flag = true;
    delay_count++;
  }
  _index++;
}

}  // namespace hitcon

void UsbServiceOnDataReceived(uint8_t* data) {
  hitcon::g_usb_logic.OnDataRecv(data);
  // TODO: queue task
}
