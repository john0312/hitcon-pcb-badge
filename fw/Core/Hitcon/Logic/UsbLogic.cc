#include <App/ShowNameApp.h>
#include <Logic/NvStorage.h>
#include <Logic/UsbLogic.h>
#include <Logic/crc32.h>
#include <Service/FlashService.h>
#include <Service/Sched/Scheduler.h>
#include <Util/uint_to_str.h>
#include <main.h>
#include <usb_device.h>
#include <usbd_custom_hid_if.h>
#include <usbd_def.h>
using namespace hitcon::service::sched;

namespace hitcon {
namespace usb {

UsbLogic g_usb_logic;

UsbLogic::UsbLogic()
    : _routine_task(810, (task_callback_t)&UsbLogic::Routine, (void*)this,
                    DELAY_INTERVAL),
      _write_routine_task(810, (task_callback_t)&UsbLogic::WriteRoutine,
                          (void*)this, WAIT_INTERVAL) {}

void UsbLogic::Init() {
  _state = USB_STATE_IDLE;
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.Queue(&_write_routine_task, nullptr);
}

// first byte of the report must be CUSTOM_REPORT_ID
void UsbLogic::OnDataRecv(uint8_t* data) {
  if (_state == USB_STATE_IDLE) {
    if (data[0] != CUSTOM_REPORT_ID) return;

    if (data[1]) _state = static_cast<usb_state_t>(data[1]);
  }

  switch (_state) {
    case USB_STATE_SET_NAME: {
      static uint8_t name_index = 0;
      nv_storage_content& content = g_nv_storage.GetCurrentStorage();
      uint8_t copy_len =
          MIN(REPORT_LEN - 1, hitcon::ShowNameApp::NAME_LEN - name_index);
      memcpy(&content.name[name_index], &data[(name_index == 0) ? 2 : 1],
             copy_len);
      name_index += copy_len;
      if (name_index >= hitcon::ShowNameApp::NAME_LEN) {
        show_name_app.SetName(const_cast<const char*>(content.name));
        _state = USB_STATE_IDLE;
        name_index = 0;
      }
      break;
    }
    case USB_STATE_ERASE:
      if (!g_flash_service.IsBusy()) {
        _state = USB_STATE_WAIT_ERASE;
        g_flash_service.ErasePage(SCRIPT_FLASH_INDEX);
      }
      scheduler.EnablePeriodic(&_write_routine_task);
      break;
    case USB_STATE_START_WRITE:
      _script_len = (data[3] << 8) | data[2];
      _state = USB_STATE_WRITING;
      _program_index = 0;
    case USB_STATE_WRITING:
      memcpy(_script_temp, data + 1, REPORT_LEN - 1);
      _new_data = true;
      scheduler.EnablePeriodic(&_write_routine_task);
      break;
    case USB_STATE_WRITE_MEM: {
      static bool _first = true;
      static WriteMemPacket packet;
      if (_first) {
        _first = false;
        memcpy(&packet.u8, data + 1, REPORT_LEN - 1);
      } else {
        _first = true;
        mem_type_t mem_type = static_cast<mem_type_t>(data[1]);
        switch (mem_type) {
          case MEM_BYTE:
            *reinterpret_cast<uint8_t*>(packet.s.addr) =
                static_cast<uint8_t>(packet.s.content);
            break;
          case MEM_HALFWORD:
            *reinterpret_cast<uint16_t*>(packet.s.addr) =
                static_cast<uint16_t>(packet.s.content);
            break;
          case MEM_WORD:
            *reinterpret_cast<uint32_t*>(packet.s.addr) = packet.s.content;
            break;
        }
        _state = USB_STATE_IDLE;
      }
      break;
    }
    case USB_STATE_READ_MEM: {
      // TODO: check read mem and write mem functionality
      ReadMemPacket packet;
      memcpy(&packet.u8, data + 1, 4);
      mem_type_t mem_type = static_cast<mem_type_t>(data[5]);
      uint8_t report[REPORT_LEN - 1] = {0};
      switch (mem_type) {
        case MEM_BYTE:
          *reinterpret_cast<uint8_t*>(report) =
              *reinterpret_cast<uint8_t*>(packet.addr);
          break;
        case MEM_HALFWORD:
          *reinterpret_cast<uint16_t*>(report) =
              *reinterpret_cast<uint16_t*>(packet.addr);
          break;
        case MEM_WORD:
          *reinterpret_cast<uint32_t*>(report) =
              *reinterpret_cast<uint32_t*>(packet.addr);
          break;
      }
      SendCustomReport(report);
      _state = USB_STATE_IDLE;
      break;
    }
    default:
      break;
  }
}

// 1. check for erase done
// 2. program the script
void UsbLogic::WriteRoutine(void* unused) {
  if (_state == USB_STATE_ERASE) {
    if (!g_flash_service.IsBusy()) {
      _state = USB_STATE_WAIT_ERASE;
      g_flash_service.ErasePage(SCRIPT_FLASH_INDEX);
    }
  } else if (_state == USB_STATE_WAIT_ERASE) {
    if (!g_flash_service.IsBusy()) {
      _state = USB_STATE_IDLE;
      scheduler.DisablePeriodic(&_write_routine_task);
      uint8_t data[] = {CODE_ACTION_DONE, 0, 0, 0, 0, 0, 0, 0};
      SendCustomReport(data);
    }
  } else if (_state == USB_STATE_WRITING) {
    if (!g_flash_service.IsBusy() && _new_data) {
      uint8_t data[] = {CODE_ACTION_DONE, 0, 0, 0, 0, 0, 0, 0};
      SendCustomReport(data);
      g_flash_service.ProgramOnly(SCRIPT_FLASH_INDEX, _program_index,
                                  reinterpret_cast<uint32_t*>(_script_temp),
                                  sizeof(_script_temp));
      if (_program_index >= _script_len + 8) {
        _state = USB_STATE_IDLE;
        scheduler.DisablePeriodic(&_write_routine_task);
      }
      _program_index += sizeof(_script_temp);
      _new_data = false;
    }
  }
}

void UsbLogic::RunScript(callback_t cb, void* arg1, callback_t err_cb,
                         void* err_arg1, bool check_crc) {
  _on_finish_cb = cb;
  _on_finish_arg1 = arg1;
  _on_err_cb = err_cb;
  _on_err_arg1 = err_arg1;
  // -1 means start running the script
  _script_index = -1;
  scheduler.EnablePeriodic(&_routine_task);

  _script_len = *reinterpret_cast<uint16_t*>(SCRIPT_LEN_ADDR);
  if (check_crc) {
    uint16_t crc_len = _script_len + 4 - _script_len % 4;
    uint32_t value =
        fast_crc32(reinterpret_cast<uint8_t*>(SCRIPT_BEGIN_ADDR), crc_len);
    if (value == *reinterpret_cast<uint32_t*>(CRC32_ADDR)) {
      StopScript();
      _on_err_cb(_on_err_arg1,
                 reinterpret_cast<void*>(const_cast<char*>(CRC_FAIL_MSG)));
    }
  }

  if (_script_len > MAX_SCRIPT_LEN) {
    StopScript();
    _on_err_cb(_on_err_arg1,
               reinterpret_cast<void*>(const_cast<char*>(EMPTY_SCRIPT_MSG)));
    return;
  }
}

void UsbLogic::StopScript() {
  if (_routine_task.IsEnabled()) scheduler.DisablePeriodic(&_routine_task);
  if (_write_routine_task.IsEnabled())
    scheduler.DisablePeriodic(&_write_routine_task);
  _state = USB_STATE_IDLE;
  SendKeyCode(0, 0);
}

// run every 20ms, handle run script
void UsbLogic::Routine(void* unused) {
  static uint8_t delay_count = 0;
  static bool send_release_flag = false;
  if (_state == USB_STATE_RETRY) {
    auto ret = USBD_CUSTOM_HID_SendReport(
        &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);
    _state = (ret != USBD_OK) ? USB_STATE_IDLE : _state;
    return;
  }
  // After sending each Keycode, send a Release
  if (send_release_flag) {
    send_release_flag = false;
    SendKeyCode(0, 0);
    if (delay_count != 0) delay_count--;
    return;
  }

  if (delay_count != 0) {
    delay_count--;
    return;
  } else if (_script_index == _script_len) {
    scheduler.DisablePeriodic(&_routine_task);
    _on_finish_cb(_on_finish_arg1, nullptr);
    return;
  }

  if (_script_index == -1) {  // new script begin
    delay_count = 0;
  } else {
    uint8_t progress = _script_index * 100 / _script_len;
    char str[4] = "XX%";
    str[0] = progress / 10 + '0';
    str[1] = progress % 10 + '0';
    display_set_mode_text(str);
    uint8_t* addr =
        reinterpret_cast<uint8_t*>(SCRIPT_BEGIN_ADDR + _script_index);
    switch (*addr) {
      case CODE_DELAY:
        _script_index++;
        delay_count = *(addr + 1) - 1;
        break;
      case CODE_RELEASE:
        SendKeyCode(0, 0);
        break;
      case CODE_MODIFIER:
        _script_index += 2;
        SendKeyCode(*(addr + 2), *(addr + 1));
        break;
      default:
        SendKeyCode(*addr, 0);
        break;
    }
    send_release_flag = true;
  }
  _script_index++;
}

void UsbLogic::SendKeyCode(uint8_t keycode, uint8_t modifier) {
  _report.report_id = KEYBOARD_REPORT_ID;
  memset(_report.u8, 0, 8);
  _report.keyboard_report.keycode[0] = keycode;
  _report.keyboard_report.modifier = modifier;
  auto ret = USBD_CUSTOM_HID_SendReport(
      &hUsbDeviceFS, reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);
  _state = (ret != USBD_OK) ? USB_STATE_IDLE : _state;
}

// TODO: add retry
void UsbLogic::SendCustomReport(uint8_t* data) {
  _report.report_id = CUSTOM_REPORT_ID;
  memcpy(_report.u8, data, REPORT_LEN - 1);
  USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                             reinterpret_cast<uint8_t*>(&_report), REPORT_LEN);
}

}  // namespace usb
}  // namespace hitcon

void UsbServiceOnDataReceived(uint8_t* data) {
  hitcon::usb::g_usb_logic.OnDataRecv(data);
}
