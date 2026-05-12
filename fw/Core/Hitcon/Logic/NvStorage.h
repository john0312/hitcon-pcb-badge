#ifndef SERVICE_NV_STORAGE_H_
#define SERVICE_NV_STORAGE_H_

#include <App/ShowNameApp.h>
#include <App/TamaApp.h>
#include <Logic/GameParam.h>
#include <Logic/GameScore.h>
#include <Service/FlashService.h>
#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

// Current NV schema version. Bump only when changing existing field layout
// (shrink, reorder, type change). Append-only growth does NOT bump this.
constexpr uint16_t NV_SCHEMA_VERSION_CURRENT = 1;

// Reserved slot count for max_scores. Locked at v1 freeze. Exceeding requires
// schema_version bump + migrator (NOT just enlarging this constant).
constexpr size_t NV_MAX_SCORE_SLOTS = 8;

// Locked total size of nv_storage_content_t. If sizeof diverges from this,
// the static_assert below fires and the developer must respond:
//   - Append (size grew): update NV_FIELDS table and NV_LAYOUT_SIZE_V1.
//   - Shrink/reorder: bump schema_version + write a migrator.
constexpr size_t NV_LAYOUT_SIZE_V1 = 80;

typedef struct nv_storage_content_t {
  // CRC32 over `content_size` bytes immediately following this field.
  uint32_t checksum;
  // Bytes after checksum that participate in CRC. Equals
  // `sizeof(nv_storage_content) - sizeof(uint32_t)` at write time. Reading
  // older shorter records: only those bytes are validated; new fields stay
  // zero-initialized in RAM.
  uint16_t content_size;
  // Schema version this record was written with. = NV_SCHEMA_VERSION_CURRENT.
  uint16_t schema_version;
  // Per-schema monotonic write counter; resets to 1 in each new schema. Used
  // to find the newest record within the same schema across round-robin pages.
  int32_t generation;

  // === Append-only payload. New fields go at the end. Never reorder, shrink,
  // or change types of existing fields without bumping schema_version. ===
  hitcon::game::game_storage_t game_storage;
  char name[hitcon::ShowNameApp::NAME_LEN + 1];
  uint32_t max_scores[hitcon::NV_MAX_SCORE_SLOTS];
  hitcon::app::tama::tama_storage_t tama_storage;
} nv_storage_content;

// === Layout invariants (X-macro) ===
// Field offset table. Append new entries; never edit existing ones.
#define NV_FIELDS(X)   \
  X(checksum, 0)       \
  X(content_size, 4)   \
  X(schema_version, 6) \
  X(generation, 8)     \
  X(game_storage, 12)  \
  X(name, 13)          \
  X(max_scores, 36)    \
  X(tama_storage, 68)

#define NV_ASSERT_OFFSET(field, expected)                            \
  static_assert(offsetof(nv_storage_content_t, field) == (expected), \
                "NV field '" #field "' offset shifted - layout broken");
NV_FIELDS(NV_ASSERT_OFFSET)
#undef NV_FIELDS
#undef NV_ASSERT_OFFSET

static_assert(sizeof(nv_storage_content) <= MY_FLASH_PAGE_SIZE,
              "nv_storage_content is too large");
static_assert(sizeof(nv_storage_content) % 4 == 0);

static_assert(sizeof(nv_storage_content) == NV_LAYOUT_SIZE_V1,
              "NV layout total size changed. "
              "Append -> update NV_FIELDS table + NV_LAYOUT_SIZE_V1. "
              "Shrink/reorder -> bump schema_version + write a migrator.");

static_assert(static_cast<size_t>(GameScoreType::GAME_UNUSED_MAX) <=
                  NV_MAX_SCORE_SLOTS,
              "Game count exceeded NV_MAX_SCORE_SLOTS budget. "
              "Bump schema_version + write a migrator.");

static_assert(ShowNameApp::NAME_LEN == 22,
              "NAME_LEN changed - resizes NV layout. "
              "Bump schema_version + write a migrator.");

// === v0 frozen layout (pre-refactor). Used only by the v0 -> v1 migrator. ===
namespace nv_v0 {

constexpr size_t V0_NAME_LEN = 22;   // frozen ShowNameApp::NAME_LEN at v1 cut
constexpr size_t V0_MAX_SCORES = 4;  // frozen GameScoreType::GAME_UNUSED_MAX

// Frozen byte-for-byte copy of game::game_storage_t at v0 freeze time.
struct game_storage_t {
  uint8_t user_id_acknowledged;
};

// Frozen byte-for-byte copy of app::tama::tama_storage_t at v0 freeze time.
struct tama_storage_t {
  uint8_t state;
  uint8_t type;
  uint8_t hp;
  uint8_t hunger;
  uint16_t qte_level;
  uint16_t step_level;
  uint32_t sponsor_register;
};

// Pre-refactor on-flash layout. Frozen forever.
typedef struct content_t {
  uint32_t checksum;
  int32_t version;
  game_storage_t game_storage;
  char name[V0_NAME_LEN + 1];
  uint32_t max_scores[V0_MAX_SCORES];
  tama_storage_t tama_storage;
} content_t;

constexpr size_t V0_SIZE = sizeof(content_t);
static_assert(V0_SIZE == 60, "v0 layout size diverged from freeze-time value");

// Freeze-time sanity checks. These are NOT permanent invariants. When a
// live struct evolves, these will fire - that is the schema-bump signal:
//   1. Bump NV_SCHEMA_VERSION_CURRENT to 2.
//   2. Snapshot the current v1 layout into nv_v1::content_t (mirroring this).
//   3. Write migrate_v1_to_v2 (and update migrate_v0_to_v1's output type).
//   4. DELETE the asserts below; they have lost meaning (v0 stays v0 forever).
static_assert(
    sizeof(game_storage_t) == sizeof(::hitcon::game::game_storage_t),
    "Live game_storage_t changed since v0 freeze. "
    "Bump schema_version, freeze v1, write migrator, then DELETE this assert.");
static_assert(
    sizeof(tama_storage_t) == sizeof(::hitcon::app::tama::tama_storage_t),
    "Live tama_storage_t changed since v0 freeze. "
    "Bump schema_version, freeze v1, write migrator, then DELETE this assert.");

}  // namespace nv_v0

// Migrate a v0 record into the current (v1) in-RAM struct. Always outputs v1
// shape. When schema_version bumps, this function's output type must change
// to nv_v1::content_t& - see static_assert in body.
void migrate_v0_to_v1(const nv_v0::content_t& old, nv_storage_content& neu);

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
  // (schema_version, generation) as the current storage. If a v0 page is the
  // best candidate, lazily migrate it. If no valid page is found, init a new
  // nv_storage_content with schema_version=NV_SCHEMA_VERSION_CURRENT,
  // generation=1.
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
  int last_flush_cycle = 0;

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
