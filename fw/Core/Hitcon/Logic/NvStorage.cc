#include <Logic/NvStorage.h>
#include <Logic/crc32.h>
#include <Service/FlashService.h>
#include <Service/Sched/Checks.h>
#include <Service/Sched/Scheduler.h>
#include <main.h>
#include <string.h>

#include <algorithm>

using hitcon::service::sched::my_assert;
using hitcon::service::sched::scheduler;

namespace hitcon {

// How long between each Flash write? In units of 0.1s.
constexpr int kMinFlushInterval = 500;

// Bytes after the leading checksum that participate in CRC for each schema.
constexpr size_t kV1PayloadSize = sizeof(nv_storage_content) - sizeof(uint32_t);
constexpr size_t kV0PayloadSize = sizeof(nv_v0::content_t) - sizeof(uint32_t);

// Minimum acceptable content_size: the CRC must cover at least content_size,
// schema_version, and generation themselves, otherwise those header fields
// are not integrity-protected. = offset of the first payload field minus
// the leading checksum.
constexpr size_t kV1MinPayloadSize =
    offsetof(nv_storage_content_t, game_storage) - sizeof(uint32_t);
static_assert(kV1MinPayloadSize == 8);

NvStorage g_nv_storage;

NvStorage::NvStorage()
    : routine_task(800, CB_CAST(&NvStorage::Routine), this, 100) {}

namespace {

bool ValidateV1(const uint8_t* page_data) {
  // This function validates v1 specifically. While NV_SCHEMA_VERSION_CURRENT
  // is 1, dereferencing as nv_storage_content is correct (it IS v1's shape).
  // When schema bumps, refactor: snapshot v1 into nv_v1::content_t and change
  // the cast below to use nv_v1::content_t.
  static_assert(NV_SCHEMA_VERSION_CURRENT == 1,
                "ValidateV1 dereferences as nv_storage_content assuming it "
                "is v1's shape. When bumping schema, snapshot v1 into "
                "nv_v1::content_t and change the cast in this function.");

  const nv_storage_content* p =
      reinterpret_cast<const nv_storage_content*>(page_data);
  if (p->content_size < kV1MinPayloadSize || p->content_size > kV1PayloadSize) {
    return false;
  }
  // content_size must be multiple of 4 for CRC32
  if (p->content_size % 4 != 0) return false;
  if (p->schema_version != 1) return false;
  if (p->generation <= 0) return false;
  uint32_t computed = fast_crc32(page_data + sizeof(uint32_t), p->content_size);
  return computed == p->checksum;
}

bool ValidateV0(const uint8_t* page_data) {
  const nv_v0::content_t* p =
      reinterpret_cast<const nv_v0::content_t*>(page_data);
  if (p->version <= 0) return false;
  uint32_t computed = fast_crc32(page_data + sizeof(uint32_t), kV0PayloadSize);
  return computed == p->checksum;
}

}  // namespace

void migrate_v0_to_v1(const nv_v0::content_t& old, nv_storage_content& neu) {
  // This migrator always outputs the v1 shape. nv_storage_content is v1 only
  // while NV_SCHEMA_VERSION_CURRENT == 1. When schema bumps, snapshot v1 into
  // nv_v1::content_t and change this function's output type to
  // nv_v1::content_t&.
  static_assert(NV_SCHEMA_VERSION_CURRENT == 1,
                "migrate_v0_to_v1 writes to nv_storage_content assuming it is "
                "v1's shape. When bumping schema, snapshot v1 into "
                "nv_v1::content_t and change this migrator's output type to "
                "nv_v1::content_t&.");

  memset(&neu, 0, sizeof(neu));
  neu.content_size = kV1PayloadSize;
  neu.schema_version = 1;
  neu.generation = 1;

  neu.game_storage.user_id_acknowledged = old.game_storage.user_id_acknowledged;

  static_assert(nv_v0::V0_NAME_LEN == ShowNameApp::NAME_LEN);
  memcpy(neu.name, old.name, sizeof(neu.name));

  constexpr size_t kCopy = std::min(NV_MAX_SCORE_SLOTS, nv_v0::V0_MAX_SCORES);
  for (size_t i = 0; i < kCopy; i++) {
    neu.max_scores[i] = old.max_scores[i];
  }

  neu.tama_storage.state =
      static_cast<app::tama::TAMA_APP_STATE>(old.tama_storage.state);
  neu.tama_storage.type =
      static_cast<app::tama::TAMA_TYPE>(old.tama_storage.type);
  neu.tama_storage.hp = old.tama_storage.hp;
  neu.tama_storage.hunger = old.tama_storage.hunger;
  neu.tama_storage.qte_level = old.tama_storage.qte_level;
  neu.tama_storage.step_level = old.tama_storage.step_level;
  neu.tama_storage.sponsor_register = old.tama_storage.sponsor_register;
}

void NvStorage::Init() {
  my_assert(!g_flash_service.IsBusy());

  // Track best candidate by (schema_version DESC, generation DESC). v0 pages
  // are treated as schema_version=0 so v1 always beats v0 even when the v1
  // generation is small (e.g. just-migrated generation=1 vs old v0 version=N).
  bool found_any = false;
  bool best_is_v0 = false;
  int32_t best_schema = -1;
  int32_t best_generation = -1;
  size_t best_page = 0;

  for (size_t i = 1; i < FLASH_PAGE_COUNT; i++) {
    uint8_t* page_data =
        reinterpret_cast<uint8_t*>(g_flash_service.GetPagePointer(i));

    int32_t this_schema;
    int32_t this_generation;
    bool this_is_v0;

    if (ValidateV1(page_data)) {
      const nv_storage_content* p =
          reinterpret_cast<const nv_storage_content*>(page_data);
      this_schema = p->schema_version;
      this_generation = p->generation;
      this_is_v0 = false;
    } else if (ValidateV0(page_data)) {
      const nv_v0::content_t* p =
          reinterpret_cast<const nv_v0::content_t*>(page_data);
      this_schema = 0;
      this_generation = p->version;
      this_is_v0 = true;
    } else {
      continue;
    }

    bool better =
        !found_any || this_schema > best_schema ||
        (this_schema == best_schema && this_generation > best_generation);
    if (better) {
      found_any = true;
      best_is_v0 = this_is_v0;
      best_schema = this_schema;
      best_generation = this_generation;
      best_page = i;
    }
  }

  if (found_any) {
    uint8_t* page_data =
        reinterpret_cast<uint8_t*>(g_flash_service.GetPagePointer(best_page));
    if (best_is_v0) {
      const nv_v0::content_t* p =
          reinterpret_cast<const nv_v0::content_t*>(page_data);
      migrate_v0_to_v1(*p, content_);
      // Mark dirty so the next flush writes the migrated content in v1 format.
      storage_dirty_ = true;
    } else {
      const nv_storage_content* p =
          reinterpret_cast<const nv_storage_content*>(page_data);
      // Zero-init first so any fields beyond the stored content_size (i.e.,
      // appended in newer firmware than what wrote this page) get their
      // default of 0.
      memset(&content_, 0, sizeof(content_));
      memcpy(&content_, p, p->content_size + sizeof(uint32_t));
      // If the loaded record was shorter than the current layout (older
      // firmware wrote it before fields were appended), normalize content_size
      // and mark dirty so the next flush rewrites a full-size record. The
      // appended fields are already zero-initialized in content_. When the
      // condition is false the loaded value already equals kV1PayloadSize and
      // flash already holds a current-layout record, so we deliberately do
      // NOT mark dirty - flushing identical bytes would only wear the flash.
      if (content_.content_size < kV1PayloadSize) {
        content_.content_size = kV1PayloadSize;
        storage_dirty_ = true;
      }
    }
    next_available_page = (best_page + 1) % FLASH_PAGE_COUNT;
  } else {
    memset(&content_, 0, sizeof(content_));
    content_.content_size = kV1PayloadSize;
    content_.schema_version = NV_SCHEMA_VERSION_CURRENT;
    content_.generation = 1;
    storage_dirty_ = true;
    force_flush = true;
  }

  storage_valid_ = true;
  // Ensure the next flush writes a strictly higher generation than any
  // existing same-schema page on flash, avoiding ties on reboot.
  content_.generation++;
  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void NvStorage::ForceFlushInternal() {
  if (!storage_dirty_) return;
  if (g_flash_service.IsBusy()) return;
  memcpy(&write_buffer_, &content_, sizeof(nv_storage_content));
  write_buffer_.content_size = kV1PayloadSize;
  write_buffer_.schema_version = NV_SCHEMA_VERSION_CURRENT;
  write_buffer_.checksum =
      fast_crc32(reinterpret_cast<uint8_t*>(&write_buffer_) + sizeof(uint32_t),
                 write_buffer_.content_size);
  // page 0 is reserved for badusb script
  if (next_available_page == 0) next_available_page++;
  bool ret = g_flash_service.ProgramPage(
      next_available_page, reinterpret_cast<uint32_t*>(&write_buffer_),
      sizeof(nv_storage_content));
  if (ret) {
    storage_dirty_ = false;
    next_available_page = (next_available_page + 1) %
                          FLASH_PAGE_COUNT;  // Increment for the next write
    content_.generation++;
    last_flush_cycle = current_cycle;  // Record the current cycle
  }
}

void NvStorage::ForceFlush(callback_t on_done, void* callback_arg1) {
  force_flush = true;
  on_done_cb = on_done;
  on_done_cb_arg1 = callback_arg1;
}

void NvStorage::Routine(void* unused) {
  current_cycle++;

  if (on_done_cb && !force_flush && !g_flash_service.IsBusy()) {
    on_done_cb(on_done_cb_arg1, nullptr);
    on_done_cb = nullptr;
  }

  if ((force_flush || current_cycle - last_flush_cycle >= kMinFlushInterval)) {
    // Programming/Erasing within the first 3s may cause issues.
    if (current_cycle >= 30) {
      ForceFlushInternal();
      force_flush = false;
    }
  }
}

}  // namespace hitcon
