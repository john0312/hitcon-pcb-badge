#ifndef LOGIC_GAME_DOT_H_
#define LOGIC_GAME_DOT_H_

#include <stddef.h>
#include <stdint.h>
#include <utility>

#include <Service/Sched/Scheduler.h>
#include <Logic/GameParam.h>
#include <Logic/keccak.h>
#include <Util/CircularQueue.h>

namespace hitcon {
namespace game {

// Implements the natural log ln() with approximate polynomial.
// The output is in Q9.22 fixed point number, while the input is in
// uint32_t integer format.
int32_t q22_ln(uint32_t) ;

class GameLogic {
 public:
  GameLogic();

  // Initialize the game. If storage is NULL, generate a new one.
  void Init(game_storage_t *storage = nullptr);

  // Accept data received from a peer or anywhere else.
  // Generally this queues into queue_ and let Routine() handle the rest.
  // Only fails if the queue is full.
  bool AcceptData(int col, uint8_t *data);

  // Retrieve the data from a specific cell in the game grid.
  uint8_t* GetDataCell(int col, int row);

  // Get random data cell for IR transmission. Writes data to out_data and column to out_col.
  bool GetRandomDataForIrTransmission(uint8_t* out_data, int* out_col);

 private:
  game_storage_t* storage_;
  game_cache_t cache_;

  enum RoutineState {
    // Sets the cell data to be random.
    // Each run of Routine() in this state populates one column, and uses
    // in_progress_col_.
    RANDOM_INIT_CELLS,
    // In this phase, Routine iterates through every cell in storage_ and
    // populates the corresponding cell_score_cache. After every cell in
    // cell_score_cache is populated, it'll move onto the next phase.
    // 1 cell is populated every 5 iteration, see RoutineSubState for more
    // information.
    POPULATING_CACHE_CELLS,
    // In this phase, Routine iterates through each column and populate the
    // col_score_cache. One column is populated per iteration.
    // It moves on after every column is computed.
    POPULATING_CACHE_COLS,
    // In this phase the final score is computed.
    POPULATING_CACHE_SCORE,
    // In this phase the routine state is waiting for any incoming data
    // inserted by AcceptData. If one is detected it'll move to the next state.
    WAITING_FOR_DATA,
    // In this phase the score for the incoming data from queue is computed
    // for the corresponding column. This phase takes 5 iteration, see
    // RoutineSubState for more information.
    COMPUTING_INCOMING_DATA,
    // In this phase we scan through the entire column for the given data
    // and see if any row has score that's lower than incoming data's score
    // If so, remove the lowest score item and replace with the incoming data.
    UPDATE_GAME_DATA,
    // In this phase we compute the score for the updated column, and also
    // the final score.
    UPDATE_GAME_SCORE
  };

  RoutineState routine_state_;

  enum RoutineSubState {
    // In this sub state, we initialize the sha3 state.
    INIT_SHA3,
    // In this sub state, we run UpdateWord() on the column prefix.
    UPDATE_COL_PREFIX,
    // In this sub state, we run UpdateWord() on the actual cell data.
    UPDATE_CELL_DATA,
    // In this sub state, we run Finalize() on the sha3 state.
    FINALIZE_SHA3,
    // In this sub state, we comute the number of prefix 0 bit in the output.
    COMPUTE_SCORE
  };

  RoutineSubState routine_sub_state_;

  // The contet for the current in-progress SHA3 256 computation.
  sha3_context routine_sha3_256_context_;

  const uint8_t* routine_sha3_hash_out_;

  // Which column are we populating in POPULATING_CACHE_CELLS state?
  int populating_cache_col_;
  // Which row are we populating in POPULATING_CACHE_CELLS state?
  int populating_cache_row_;

  // Which column's score are we computing in POPULATING_CACHE_COLS state?
  int populating_col_score_;

  // The data and col that we just poped from the accept data queue.
  int in_progress_col_;
  grid_cell_t in_progress_data_;

  // Computed in progress score for processing of AcceptData.
  int in_progress_cell_score_;

  CircularQueue<std::pair<int, grid_cell_t>, kQueueSize> queue_;

  hitcon::service::sched::PeriodicTask routine_task;

  // A simple helper function that sets out to the column prefix for the
  // given column. It'll be set to the value of "HITCON" and the last 2 byte
  // is the MSB and LSB of uint16_t of col.
  void SetColumnPrefix(uint8_t *out, int col);

  // A simple routine function that attempts to populate the cache and handle
  // any items in the AcceptData queue.
  void Routine();

  // Iterate through the sub routine enums one step per call.
  // If we just finished the last step in the sub routine, the we'll return true
  // and out_score is set to the score of the give col/cell_data.
  // Otherwise, false is returned and we should call StepSubRoutine() again in
  // the next Routine invocation.
  bool StepSubRoutine(int col, uint8_t* cell_data, int* out_score);

  // Computes the number of prefix 0 bit in the given bin_hash, which is a hash
  // of sha3 256 bit variant.
  int ComputePrefixZero(const uint8_t* bin_hash);

  // Computes the columns's score from all the cell in the column.
  // Updates the cache after the computation.
  void ComputeColumnScore(int col);

  // Computes the final score from the column score.
  // Updates the cache after the computation.
  void ComputeFinalScore();

  void RandomlySetGridCellValue(int row, int col);
};

extern GameLogic gameLogic;

}  // namespace hitcon
}  // namespace game

#endif // LOGIC_GAME_DOT_H_
