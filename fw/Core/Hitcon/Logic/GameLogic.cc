
#include <Logic/EntropyHub.h>
#include <Logic/GameLogic.h>
#include <Logic/NvStorage.h>
#include <Logic/RandomPool.h>
#include <Logic/keccak.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/SysTimer.h>

#include <cstring>
#include <limits>

namespace hitcon {
namespace game {

using hitcon::service::sched::my_assert;
using hitcon::service::sched::scheduler;
using hitcon::service::sched::SysTimer;

constexpr unsigned kIdleRoutineDelay = 20;

GameLogic gameLogic;

void GameLogic::RandomlySetGridCellValue(int row, int col) {
  for (uint8_t i = 0; i < kDataSize; i++) {
    storage_->cells[col][row].data[i] = g_fast_random_pool.GetRandom();
  }
}

GameLogic::GameLogic()
    : routine_task_now(1000, (callback_t)&GameLogic::Routine, this),
      routine_task_delayed(1000, (callback_t)&GameLogic::Routine, this, 0),
      random_data_count_(0), accept_data_count_(0), accepted_data_count_(0),
      game_ready(false) {}

void GameLogic::Init(game_storage_t *storage) {
  storage_ = storage;

  // if empty, generate random data
  routine_state_ = CHECK_CELLS_VALID;
  populating_cache_col_ = populating_cache_row_ = 0;
  memset(&cache_, 0, sizeof(game_cache_t));
  scheduler.Queue(&routine_task_now, nullptr);
}

bool GameLogic::AcceptData(int col, uint8_t *data) {
  // TODO: schedule task deal with accepting data
  // game_queue.push(col, data);
  my_assert(col >= 0 && col < kNumCols);
  std::pair<int, grid_cell_t> p;
  p.first = col;
  memcpy(&p.second.data[0], data, sizeof(p.second));
  return queue_.PushBack(p);
}

void GameLogic::DoRandomData() {
  int col = InternalGenCol[g_fast_random_pool.GetRandom() % InternalGenColCnt];
  uint8_t rdata[kDataSize];
  for (int i = 0; i < kDataSize; i++) {
    rdata[i] = g_fast_random_pool.GetRandom() & 0x0FF;
  }
  AcceptData(col, rdata);
  random_data_count_++;
}

game_cache_t &GameLogic::get_cache() { return cache_; };

void GameLogic::GetRandomDataForIrTransmission(uint8_t *out_data,
                                               int *out_col) {
  int col = IrAllowedBroadcastCol[g_fast_random_pool.GetRandom() %
                                  IrAllowedBroadcastColCnt];
  int row = g_fast_random_pool.GetRandom() % kNumRows;
  *out_col = col;
  memcpy(out_data, GetDataCell(col, row), kDataSize);
}

void GameLogic::GetRandomDataForXBoardTransmission(uint8_t *out_data,
                                                   int *out_col) {
  int col = XBoardAllowedBroadcastCol[g_fast_random_pool.GetRandom() %
                                      XBoardAllowedBroadcastColCnt];
  int row = g_fast_random_pool.GetRandom() % kNumRows;
  *out_col = col;
  memcpy(out_data, GetDataCell(col, row), kDataSize);
}

void GameLogic::SetColumnPrefix(uint8_t *out, int col) {
  // Copy "HITCON" to the output buffer
  memcpy(out, "HITCON", 6);

  // Convert col to uint16_t and split it into MSB and LSB
  uint16_t col_uint16 = static_cast<uint16_t>(col);
  uint8_t msb = (col_uint16 >> 8) & 0xFF;  // Most significant byte
  uint8_t lsb = col_uint16 & 0xFF;         // Least significant byte

  // Set the last 2 bytes of the output buffer to the MSB and LSB of col
  out[6] = msb;
  out[7] = lsb;
}

namespace {
// Look-up Table for the number of leading zero bits in a nibble
constexpr int LEADING_ZERO_BITS_LUT[16] = {4, 3, 2, 2, 1, 1, 1, 1,
                                           0, 0, 0, 0, 0, 0, 0, 0};
}  // namespace

// Computes the number of leading zero bits in the given binary hash
int GameLogic::ComputePrefixZero(const uint8_t *bin_hash) {
  int count = 0;
  for (int i = 0; i < SHA3_256_HASH_SIZE; i++) {
    uint8_t byte = bin_hash[i];
    uint8_t high_nibble = (byte & 0xF0) >> 4;
    if (high_nibble != 0) {
      return count + LEADING_ZERO_BITS_LUT[high_nibble];
    }
    count += 4;

    uint8_t low_nibble = byte & 0x0F;
    if (low_nibble != 0) {
      return count + LEADING_ZERO_BITS_LUT[low_nibble];
    }
    count += 4;
  }
  // All bytes are zero, so return the total number of bits in the hash.
  // return SHA3_256_HASH_SIZE * 8;
  // Ah screw it, nobody gets here, might as well **** around.
  return *reinterpret_cast<int *>(0);
}

bool GameLogic::StepSubRoutine(int col, uint8_t *cell_data, int *out_score) {
  switch (routine_sub_state_) {
    case INIT_SHA3:
      sha3_Init256(&routine_sha3_256_context_);
      routine_sub_state_ = UPDATE_COL_PREFIX;
      return false;

    case UPDATE_COL_PREFIX: {
      uint8_t col_prefix[kColPrefixLen];
      SetColumnPrefix(col_prefix, col);
      sha3_UpdateWord(&routine_sha3_256_context_, col_prefix);
      routine_sub_state_ = UPDATE_CELL_DATA;
      return false;
    }

    case UPDATE_CELL_DATA:
      sha3_UpdateWord(&routine_sha3_256_context_, cell_data);
      routine_sub_state_ = FINALIZE_SHA3;
      sha3_finalize_round_ = 0;
      return false;

    case FINALIZE_SHA3: {
      routine_sha3_hash_out_ =
          reinterpret_cast<const uint8_t *>(sha3_Finalize_split(
              &routine_sha3_256_context_, sha3_finalize_round_));
      sha3_finalize_round_++;
      if (routine_sha3_hash_out_) {
        routine_sub_state_ = COMPUTE_SCORE;
      }
      return false;
    }

    case COMPUTE_SCORE:
      *out_score = ComputePrefixZero(
          reinterpret_cast<const uint8_t *>(routine_sha3_hash_out_));
      routine_sub_state_ = INIT_SHA3;
      return true;

    default:
      // Invalid sub-state
      return false;
  }
}

void GameLogic::ComputeColumnScore(int col) {
  my_assert(col >= 0 && col < kNumCols);
  uint32_t column_score = 0;
  for (size_t row = 0; row < kNumRows; ++row) {
    uint32_t cell_score =
        static_cast<uint32_t>(cache_.cell_score_cache[col][row]);
    column_score += cell_score * cell_score;
  }
  cache_.col_score_cache[col] = column_score;
}

void GameLogic::ComputeFinalScore() {
  cache_.total_score = 0;
  for (int col = 0; col < kNumCols; col++) {
    if (cache_.col_score_cache[col] > 0) {
      cache_.total_score += cache_.col_score_cache[col];
    }
  }
}

void GameLogic::Routine() {
  bool idle = RoutineInternal();
  if (idle) {
    routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kIdleRoutineDelay);
    scheduler.Queue(&routine_task_delayed, nullptr);
  } else {
    scheduler.Queue(&routine_task_now, nullptr);
  }
}

bool GameLogic::RoutineInternal() {
  bool idle = false;
  switch (routine_state_) {
    case CHECK_CELLS_VALID: {
      // Wait until random is ready.
      if (!g_entropy_hub.EntropyReady()) {
        idle = true;
        break;
      }
      bool all_zero = true;
      for (size_t row = 0; row < kNumRows; ++row) {
        for (size_t byte = 0; byte < kDataSize; ++byte) {
          if (storage_->cells[populating_cache_col_][row].data[byte] != 0) {
            all_zero = false;
            break;
          }
        }
        if (!all_zero) break;
      }

      if (!all_zero) {
        routine_state_ = POPULATING_CACHE_CELLS;
        routine_sub_state_ = INIT_SHA3;
        populating_cache_col_ = 0;
        populating_cache_row_ = 0;
        break;
      }

      // Move to the next column for the next run
      populating_cache_col_ = (populating_cache_col_ + 1) % kNumCols;
      if (populating_cache_col_ == 0) {
        routine_state_ = RANDOM_INIT_CELLS;
        in_progress_col_ = 0;
      }
      break;
    }
    case RANDOM_INIT_CELLS: {
      // Set the cell data to be random for the current column.
      for (size_t row = 0; row < kNumRows; ++row) {
        RandomlySetGridCellValue(row, in_progress_col_);
      }
      // Move to the next column.
      in_progress_col_ = (in_progress_col_ + 1) % kNumCols;
      // If all columns have been initialized, move to the next state.
      if (in_progress_col_ == 0) {
        g_nv_storage.MarkDirty();
        routine_state_ = POPULATING_CACHE_CELLS;
        populating_cache_col_ = 0;
        populating_cache_row_ = 0;
        routine_sub_state_ = INIT_SHA3;
        break;
      }
      break;
    }
    case POPULATING_CACHE_CELLS: {
      int score;
      bool sub_routine_complete = StepSubRoutine(
          populating_cache_col_,
          storage_->cells[populating_cache_col_][populating_cache_row_].data,
          &score);
      if (sub_routine_complete) {
        // If the sub routine is complete, update the cell score cache.
        cache_.cell_score_cache[populating_cache_col_][populating_cache_row_] =
            score;
        // Move to the next cell.
        populating_cache_row_ = (populating_cache_row_ + 1) % kNumRows;
        // If all cells in the current column have been processed, move to the
        // next column.
        if (populating_cache_row_ == 0) {
          populating_cache_col_ = (populating_cache_col_ + 1) % kNumCols;
        }
        // If all columns have been processed, move to the next state.
        if (populating_cache_col_ == 0 && populating_cache_row_ == 0) {
          routine_state_ = POPULATING_CACHE_COLS;
          populating_col_score_ = 0;
        }
      }
      break;
    }
    case POPULATING_CACHE_COLS: {
      ComputeColumnScore(populating_col_score_);
      populating_col_score_ = (populating_col_score_ + 1) % kNumCols;
      if (populating_col_score_ == 0) {
        // If all columns have been processed, move to the next state.
        routine_state_ = POPULATING_CACHE_SCORE;
      }
      break;
    }
    case POPULATING_CACHE_SCORE: {
      ComputeFinalScore();
      routine_state_ = WAITING_FOR_DATA;
      break;
    }
    case WAITING_FOR_DATA: {
      // Check if any incoming data has been inserted into the accept data
      // queue.
      game_ready = true;
      if (!queue_.IsEmpty()) {
        // Dequeue data and begin processing.
        std::pair<int, grid_cell_t> data_pair = queue_.Front();
        queue_.PopFront();
        in_progress_col_ = data_pair.first;
        in_progress_data_ = data_pair.second;
        routine_state_ = COMPUTING_INCOMING_DATA;
        accept_data_count_++;
      } else {
        idle = true;
      }
      break;
    }
    case COMPUTING_INCOMING_DATA: {
      int score;
      bool sub_routine_complete =
          StepSubRoutine(in_progress_col_, in_progress_data_.data, &score);
      if (sub_routine_complete) {
        in_progress_cell_score_ = score;
        routine_state_ = UPDATE_GAME_DATA;
      }
      break;
    }
    case UPDATE_GAME_DATA: {
      int min_score = std::numeric_limits<int>::max();
      int min_row = -1;
      // Find the lowest score item in the column for the given data.
      for (size_t row = 0; row < kNumRows; ++row) {
        int cell_score = cache_.cell_score_cache[in_progress_col_][row];
        if (cell_score < min_score) {
          min_score = cell_score;
          min_row = row;
        }
      }
      // Replace the lowest score item with the incoming data, if its score is
      // higher.
      if (in_progress_cell_score_ > min_score) {
        my_assert(min_row != -1 && min_row < kNumRows);
        storage_->cells[in_progress_col_][min_row] = in_progress_data_;
        cache_.cell_score_cache[in_progress_col_][min_row] =
            in_progress_cell_score_;
        g_nv_storage.MarkDirty();
        accepted_data_count_++;
        routine_state_ = UPDATE_GAME_SCORE;
      } else {
        routine_state_ = WAITING_FOR_DATA;
        populating_cache_col_ = 0;
        populating_cache_row_ = 0;
      }
      break;
    }
    case UPDATE_GAME_SCORE: {
      ComputeColumnScore(in_progress_col_);
      ComputeFinalScore();
      routine_state_ = WAITING_FOR_DATA;
      populating_cache_col_ = 0;
      populating_cache_row_ = 0;
      break;
    }
  }
  return idle;
}

int GameLogic::GetScore() { return (cache_.total_score >> 4) + 1; }

int GameLogic::GetAcceptDataQueueAvailable() {
  int res =
      static_cast<int>(queue_.Capacity()) - 1 - static_cast<int>(queue_.Size());
  if (res < 0) res = 0;
  return res;
}

}  // namespace game
}  // namespace hitcon
