#ifndef SERVICE_FLASH_SERVICE_H_
#define SERVICE_FLASH_SERVICE_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {

constexpr size_t FLASH_PAGE_COUNT = 16;
constexpr size_t FLASH_PAGE_SIZE = 4096;

class FlashService {
 public:
  FlashService();

  // Init the FlashService.
  void Init();

  // Return the pointer to read a page.
  // The pointer is only valid if IsPageReadable() returns true.
  // page_id should be between 0 to FLASH_PAGE_COUNT-1.
  uint8_t* GetPagePointer(size_t page_id);

  // Return whether the page is readable, this usually whether we're busy
  // writing to that page.
  // page_id should be between 0 to FLASH_PAGE_COUNT-1.
  bool IsPageReadable(size_t page_id);

  // Whether we're writing to a page. If this returns false, then we can
  // program the next page.
  bool IsBusy();

  // Program the page page_id with data and len.
  // len must be lesser than FLASH_PAGE_SIZE.
  // Any flash storage after len remains 0.
  bool ProgramPage(size_t page_id, uint8_t* data, size_t len);

};

}  // namespace hitcon

#endif  // SERVICE_FLASH_SERVICE_H_
