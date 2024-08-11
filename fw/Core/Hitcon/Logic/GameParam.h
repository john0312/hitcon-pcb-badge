#ifndef LOGIC_GAME_PARAM_DOT_H_
#define LOGIC_GAME_PARAM_DOT_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {
namespace game {

// We've a grid of cells, each cell has some data.

constexpr size_t kNumCols = 8;
constexpr size_t kNumRows = 16;
constexpr size_t kDataSize = 8;

// We do a random generation once every kRandomGenPeriodMs ms.
constexpr size_t kRandomGenPeriodMs = 100;

typedef int score_t;

// Represents one cell in the grid.
typedef struct {
  uint8_t data[kDataSize];
} grid_cell_t;

// game_storage_t is persisted to the persistent storage.
typedef struct {
  grid_cell_t cells[kNumCols][kNumRows];
} game_storage_t;

// game_cache_t is a temporary structure that contains frequently used data,
// but is not persisted to the storage.
typedef struct {
  // The score for each cell is the number of prefix 0 bit in
  // sha3_256(col_prefix | cell_data).
  // Technically a uint8_t might overflow, but by then we're all screwed so
  // whatever.
  uint8_t cell_score_cache[kNumCols][kNumRows];

  // The score for each column is the sum of all score in the column.
  uint32_t col_score_cache[kNumCols];

  // The total score is the sum of log(col_score), where by log is the natural
  // log. This number is stored in Q format fixed number. Q9.22 is used.
  uint32_t total_score;
} game_cache_t;

// Column prefix is 8 bytes.
constexpr size_t kColPrefixLen = 8;

// These columns may be broadcasted by the IrController.
constexpr int IrAllowedBroadcastCol[] = {0, 2, 3, 4, 7};
constexpr size_t IrAllowedBroadcastColCnt =
    sizeof(IrAllowedBroadcastCol) / sizeof(IrAllowedBroadcastCol[0]);

// These columns may be transmitted through XBoard.
constexpr int XBoardAllowedBroadcastCol[] = {5, 7, 8};
constexpr size_t XBoardAllowedBroadcastColCnt =
    sizeof(XBoardAllowedBroadcastCol) / sizeof(XBoardAllowedBroadcastCol[0]);

// These columns may accept data through internal random generation.
constexpr int InternalGenCol[] = {0, 1, 2};
constexpr size_t InternalGenColCnt =
    sizeof(InternalGenCol) / sizeof(InternalGenCol[0]);

// circular queue size
constexpr size_t kQueueSize = 16;
constexpr size_t kInternalGenMinQueueAvailable = 12;
constexpr uint32_t kInternalGenChance = 5000;

}  // namespace game
}  // namespace hitcon

#endif  // LOGIC_GAME_PARAM_DOT_H_
