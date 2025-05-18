#ifndef SERVICE_NV_STORAGE_H_
#define SERVICE_NV_STORAGE_H_

#include <App/ShowNameApp.h>
#include <App/tama_src/TamaApp.h>
#include <Logic/GameParam.h>
#include <Logic/GameScore.h>
#include <Service/FlashService.h>
#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

typedef struct nv_storage_content_t {
  // This is crc32 of whatever data after this.
  uint32_t checksum;
  // Starts at 1 and increments after every page program.
  int32_t version;

  // Add any needed data that needs to be persisted here.
  hitcon::game::game_storage_t game_storage;
  char name[hitcon::ShowNameApp::NAME_LEN + 1];

  uint32_t
      max_scores[static_cast<size_t>(hitcon::GameScoreType::GAME_UNUSED_MAX)];

  // Tama Game data
  hitcon::app::tama::tama_storage_t tama_storage;
} nv_storage_content;

static_assert(sizeof(nv_storage_content) <= MY_FLASH_PAGE_SIZE,
              "nv_storage_content is too large");
static_assert(sizeof(nv_storage_content) % 4 == 0);

// This class manages the nv/flash storage, it handles the flushing/write and
// read of the persistent data. A level lower than this class is the
// FlashService class, on every flush, this class will pick a new page in a
// round robin manner and write the data into that page. When writing/flush,
// it'll also handle the crc32 computation.
class NvStorage {
 public:
  NvStorage();

  // Start the NV Storage service, this will parse all pages from FlashService
  // and find if there's a valid page, and if so, retain the newest page by
  // version as the current storage. If no valid page is found, init a new
  // nv_storage_content_t with version=1.
  void Init();

  // Return false if we've not decoded a valid NV storage.
  // ie. Init() has not finished.
  bool IsStorageValid() { return storage_valid_; }

  // Get the newest version of the storage for reading or writing.
  nv_storage_content& GetCurrentStorage() { return content_; }

  // If result of GetCurrentStorage() is modified by the caller, the caller
  // should call this function to mark the current content as dirty and needs
  // to be flushed back to flash.
  void MarkDirty() { storage_dirty_ = true; }

  // Force a write to the storage.
  // Use this sparingly as it causes flash to wear out.
  // on_done is invoked once flush is finished.
  void ForceFlush(callback_t on_done, void* callback_arg1);

  // This is called routinely every 100ms.
  void Routine(void* unused);

 private:
  void ForceFlushInternal();

  // Set to true if the current storage is a validly decoded storage content.
  bool storage_valid_;

  // The in ram copy of the storage.
  nv_storage_content content_;
  nv_storage_content write_buffer_;

  // True if content_ is dirty and should be flushed.
  bool storage_dirty_;

  // Starts at 1, incremented every Routine() call, used to time flush.
  // We automatically flush/write to lower level every 30s.
  int current_cycle;

  // Cycle at last flush.
  int last_flush_cycle;

  // Next available page to write.
  int next_available_page;

  // If we've been instructed to force flush.
  bool force_flush;

  // The callback when force flush is done.
  callback_t on_done_cb;
  void* on_done_cb_arg1;

  hitcon::service::sched::PeriodicTask routine_task;
};
extern NvStorage g_nv_storage;
}  // namespace hitcon

#endif  // #ifndef SERVICE_NV_STORAGE_H_
