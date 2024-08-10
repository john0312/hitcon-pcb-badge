#ifndef SERVICE_FLASH_SERVICE_H_
#define SERVICE_FLASH_SERVICE_H_

#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

constexpr size_t FLASH_PAGE_COUNT = 16;
constexpr size_t FLASH_END_ADDR = 0x0801FFFF;
constexpr size_t MY_FLASH_PAGE_SIZE = 0x800U;
// actually 0x400U, so it would cost 32 pages

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
  size_t _data_len;

  size_t _page_id;
  uint32_t* _data;
  size_t _addr;
  size_t _erase_page_id;
  size_t _program_page_id;
  size_t _program_pending_count_;
  size_t _wait_cnt;

  enum FlashServiceState {
    FS_IDLE,
    FS_UNLOCK,
    FS_SUSPEND_WAIT,
    FS_ERASE,
    FS_ERASE_WAIT,
    FS_RESUME_WAIT,
    FS_PROGRAM,
    FS_PROGRAM_WAIT,
  };

  hitcon::service::sched::PeriodicTask routine_task;

  FlashServiceState _state;

  void Routine();

  static constexpr size_t kErasePageCount = 2;
  static constexpr size_t kProgramPerRun = 32;
};

// Global singleton instance of FlashService.
extern FlashService g_flash_service;

}  // namespace hitcon

#endif  // SERVICE_FLASH_SERVICE_H_
