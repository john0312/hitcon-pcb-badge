#ifndef SERVICE_NV_STORAGE_H_
#define SERVICE_NV_STORAGE_H_

#include <stddef.h>
#include <stdint.h>

#include <Util/callback.h>

namespace hitcon {

typedef struct nv_storage_content_t {
  uint32_t checksum;
  uint32_t version;

  // Add any needed data that needs to be persisted here.
} nv_storage_content;

class NvStorage {
 public:
  NvStorage();

  void Init();

  // Return false if we've not decoded a valid NV storage.
  // ie. Init() has not finished.
  bool IsStorageValid() {
    return storage_valid_;
  }

  // Get the newest version of the storage for reading or writing.
  nv_storage_content& GetCurrentStorage() {
    return content_;
  }

  // If result of GetCurrentStorage() is modified by the caller, the caller
  // should call this function to mark the current content as dirty and needs
  // to be flushed back to flash.
  void MarkDirty() {
    storage_dirty_ = true;
  }

  // Force a write to the storage.
  // Use this sparingly as it causes flash to wear out.
  // on_done is invoked once flush is finished.
  void ForceFlush(callback_t on_done, void* callback_arg1);

 private:
  // Set to true if the current storage is a validly decoded storage content.
  bool storage_valid_;

  // The in ram copy of the storage.
  nv_storage_content content_;

  // True if content_ is dirty and should be flushed.
  bool storage_dirty_;
};

}

#endif  // #ifndef SERVICE_NV_STORAGE_H_
