#include <Logic/NvStorage.h>
#include <Service/FlashService.h>
#include <main.h>

namespace hitcon {

NvStorage::NvStorage() {}

void NvStorage::Init() {
  static_assert(sizeof(nv_storage_content) <= FLASH_PAGE_SIZE);
}

void NvStorage::ForceFlush(callback_t on_done, void* callback_arg1) {
  // TODO
}

}  // namespace hitcon
