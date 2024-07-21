#include <Service/FlashService.h>
#include <Service/Sched/Checks.h>
#include <main.h>

using namespace hitcon::service::sched;
using namespace hitcon;

void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue) {
  g_flash_service.EndOperationCallback(ReturnValue);
}

void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue) {
  my_assert(false);
}

namespace hitcon {
FlashService g_flash_service;

FlashService::FlashService() {}

void FlashService::Init() { _busy = false; }

void* FlashService::GetPagePointer(size_t page_id) {
  if (page_id >= 0 && page_id < FLASH_PAGE_COUNT) {
    size_t res_ptr =
        FLASH_END_ADDR - (FLASH_PAGE_COUNT - page_id) * FLASH_PAGE_SIZE + 1;
    return reinterpret_cast<void*>(res_ptr);
  }
  return nullptr;
}

// static
void FlashService::EndOperationCallback(uint32_t value) {
  static size_t count = 0;
  if (value != 0xFFFFFFFF) {
    count++;
    if (count == _data_len) {
      HAL_FLASH_Lock();
      _busy = false;
    }
  } else {  // Erase page finished.
    count = 0;
  }
}

bool FlashService::IsBusy() { return _busy; }

bool FlashService::ProgramPage(size_t page_id, uint32_t* data, size_t len) {
  if (page_id >= 0 && page_id < FLASH_PAGE_COUNT && len < FLASH_PAGE_SIZE) {
    size_t addr =
        reinterpret_cast<size_t>(GetPagePointer(page_id));  // begin address

    len /= 4;  // save in Word (4Bytes)

    _busy = true;
    _data_len = len;

    FLASH_EraseInitTypeDef erase_struct = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = addr,
        .NbPages = 1,
    };

    HAL_FLASH_Unlock();

    if (HAL_FLASHEx_Erase_IT(&erase_struct) != HAL_OK) {
      return false;
    }

    for (size_t i = 0; i < len; i++) {
      if (HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_WORD, addr, data[i]) != HAL_OK)
        return false;
      addr += 4;
    }

    return true;
  }
  return false;
}

}  // namespace hitcon
