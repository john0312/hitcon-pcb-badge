#ifndef LOGIC_GAME_PARAM_DOT_H_
#define LOGIC_GAME_PARAM_DOT_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {
namespace game {

// game_storage_t is persisted to the persistent storage.
typedef struct {
  uint8_t user_id_acknowledged;
} game_storage_t;

}  // namespace game
}  // namespace hitcon

#endif  // LOGIC_GAME_PARAM_DOT_H_
