#ifndef LOGIC_GAME_PARAM_DOT_H_
#define LOGIC_GAME_PARAM_DOT_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {
namespace game {

// Persisted as part of nv_storage_content. Changing this struct's layout
// (add/remove/resize/reorder fields) is a NV schema-affecting change: the
// freeze sanity assert in NvStorage.h's nv_v0 namespace will fire, and the
// expected response is to bump NV_SCHEMA_VERSION_CURRENT and write a migrator.
typedef struct {
  uint8_t user_id_acknowledged;
} game_storage_t;

}  // namespace game
}  // namespace hitcon

#endif  // LOGIC_GAME_PARAM_DOT_H_
