#ifndef SERVICE_FLASH_SERVICE_H_
#define SERVICE_FLASH_SERVICE_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {

constexpr size_t FLASH_PAGE_COUNT = 16;
constexpr size_t FLASH_END_ADDR = 0x0801FFFF;

class FlashService {
 public:
  FlashService();

  // Init the FlashService.
  void Init();

  // Return the pointer to read a page.
  // The pointer is only valid if IsPageReadable() returns true.
  // page_id should be between 0 to FLASH_PAGE_COUNT-1.
  void* GetPagePointer(size_t page_id);

  // wrapper for void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
  void EndOperationCallback(uint32_t value);

  // Whether we're writing to a page. If this returns false, then we can
  // program the next page.
  bool IsBusy();

  // Program the page page_id with data and len.
  // len must be lesser than FLASH_PAGE_SIZE.
  // Any flash storage after len remains 0.
  bool ProgramPage(size_t page_id, uint32_t* data, size_t len);
private:
  bool _busy;
  size_t _data_len;
};

// Global singleton instance of FlashService.
extern FlashService g_flash_service;

}  // namespace hitcon


#endif  // SERVICE_FLASH_SERVICE_H_
