#include <App/ShowNameApp.h>
#include <Logic/NvStorage.h>
#include <Logic/UsbLogic.h>
#include <Service/FlashService.h>
#include <Service/Sched/Scheduler.h>
#include <usb_device.h>
#include <usbd_custom_hid_if.h>

using namespace hitcon::service::sched;

/* TODO:
 * 1. check name length
 * 2. add usb on connect
 * 3. add run script progress (XX%)
 * 4. add hack api
 */

namespace hitcon {
namespace usb {

UsbLogic g_usb_logic;

UsbLogic::UsbLogic()
    : on_recv_task(823, (task_callback_t)&UsbLogic::OnDataRecv, this),
      _routine_task(810, (task_callback_t)&UsbLogic::Routine, (void*)this,
                    DELAY_INTERVAL),
      _write_routine_task(810, (task_callback_t)&UsbLogic::WriteRoutine,
                          (void*)this, WAIT_INTERVAL) {}

void UsbLogic::Init() {
  _state = USB_STATE_HEADER;
  _index = 0;
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.Queue(&_write_routine_task, nullptr);
}

void UsbLogic::OnDataRecv(void* arg) {
  uint8_t* data = reinterpret_cast<uint8_t*>(arg);
  static size_t counter = 0;
  counter++;
  if (_state == USB_STATE_ERASE) return;  // erase not finish
  for (uint8_t i = 0; i < RECV_BUF_LEN; i++) {
    _temp[i] = data[i];
  }

  if (_state == USB_STATE_HEADER) {
    if (data[0]) {
      _state = static_cast<usb_state_t>(data[0]);
      _index = 0;
    }
  }
  nv_storage_content& content = g_nv_storage.GetCurrentStorage();
  switch (_state) {
    case USB_STATE_SET_NAME:
      for (size_t i = (_index == 0); i < RECV_BUF_LEN; i++, _index++) {
        content.name[_index] = data[i];
        if (_index - 1 == hitcon::ShowNameApp::NAME_LEN) {
          show_name_app.SetName(reinterpret_cast<const char*>(content.name));
          _state = USB_STATE_HEADER;
          break;
        }
      }
      break;
    case USB_STATE_ERASE:
      g_flash_service.ErasePage(SCRIPT_FLASH_INDEX);  // TODO: check is busy
      scheduler.EnablePeriodic(&_write_routine_task);
      break;
    case USB_STATE_START_WRITE:
      // TODO: add checksum
      _script_len = (data[1] << 8) | data[2];
      memcpy(_temp, data, RECV_BUF_LEN);
      _state = USB_STATE_WRITING;
      _new_data = true;
      scheduler.EnablePeriodic(&_write_routine_task);
    case USB_STATE_WRITING:
      // it seems like first byte of data cannot be 0 so use 0xFC instead
      if (data[0] == 0xFC) data[0] = 0;
      memcpy(_temp, data, RECV_BUF_LEN);
      _new_data = true;
      break;
    default:
      break;
  }
}

void UsbLogic::WriteRoutine(void* unused) {
  if (_state == USB_STATE_ERASE) {
    if (!g_flash_service.IsBusy()) {
      _state = USB_STATE_HEADER;
      scheduler.DisablePeriodic(&_write_routine_task);
      keyboard_report = {0, 0, 0xFF, 0, 0, 0, 0, 0};
      USBD_CUSTOM_HID_SendReport(
          &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&keyboard_report), 8);
    }
  } else if (_state == USB_STATE_WRITING) {
    if (!g_flash_service.IsBusy() && _new_data) {
      keyboard_report = {0, 0, 0xFF, 0, 0, 0, 0, 0};
      USBD_CUSTOM_HID_SendReport(
          &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&keyboard_report), 8);
      g_flash_service.ProgramOnly(SCRIPT_FLASH_INDEX, _index,
                                  reinterpret_cast<uint32_t*>(_temp), 8);
      _index += 8;
      _new_data = false;
      if (_index >= _script_len + 8) {
        _state = USB_STATE_HEADER;
        scheduler.DisablePeriodic(&_write_routine_task);
      }
    }
  }
}

void UsbLogic::RunScript(callback_t cb, void* arg1) {
  _on_finish_cb = cb;
  _on_finish_arg1 = arg1;
  _index = -1;
  keyboard_report = {0, 0, 0, 0, 0, 0, 0, 0};
  empty_report = {0, 0, 0, 0, 0, 0, 0, 0};
  scheduler.EnablePeriodic(&_routine_task);
  flag = false;
}

void UsbLogic::StopScript() {
  if (_routine_task.IsEnabled()) scheduler.DisablePeriodic(&_routine_task);
  if (_write_routine_task.IsEnabled())
    scheduler.DisablePeriodic(&_write_routine_task);

  USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                             reinterpret_cast<uint8_t*>(&empty_report), 8);
}

// run every 20ms
void UsbLogic::Routine(void* unused) {
  static uint8_t delay_count = 0;
  uint16_t len = (*(uint8_t*)(SCRIPT_BEGIN_ADDR - 2) << 8) |
                 *(uint8_t*)(SCRIPT_BEGIN_ADDR - 1);
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
  } else if (_index == len) {
    scheduler.DisablePeriodic(&_routine_task);
    _on_finish_cb(_on_finish_arg1, nullptr);
    return;
  }
  if (_index == -1) {  // new script begin
    delay_count = 0;
  } else {
    uint8_t progress = _index * 100 / len;
    char str[4] = "XX%";
    str[0] = progress / 10 + '0';
    str[1] = progress % 10 + '0';
    display_set_mode_text((str));
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
}  // namespace usb
}  // namespace hitcon

void UsbServiceOnDataReceived(uint8_t* data) {
  scheduler.Queue(&hitcon::usb::g_usb_logic.on_recv_task,
                  reinterpret_cast<uint8_t*>(data));
}
