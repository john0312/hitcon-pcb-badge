#include "game.h"

#include <Logic/NvStorage.h>
#include <Logic/RandomPool.h>

namespace hitcon {
namespace game {

GameQueue game_queue;

// Implements the ln() with approximate polynomial.
// The output is in Q9.22 fixed point number, while the input is in
// uint32_t integer format.
int32_t q22_ln(uint32_t) {
  // TODO
}

game_cache_t game_cache;

grid_cell_t random_grid() {
  grid_cell_t cell;
  for (uint8_t i = 0; i < kDataSize; i++)
    cell.data[i] = g_fast_random_pool.GetRandom();
  return cell;
}

score_t grid_score(const grid_cell_t *grid) {
  // TODO
  // Note: This probably has to be reimplemented because it spans multiple
  // tasks.
  sha3_context priv;

  // TODO: split into tasks
  //if (sha3_Init256(&priv, 256) != SHA3_RETURN_OK) return 0;
  // TODO: update with col key
  // sha3_UpdateWord();
  //for (int i = 0; i < kDataSize; i++) sha3_UpdateWord(priv, grid->data[i]);

  // TODO: return what??
  //return q22_ln(priv);
}

static void init_game_storage() {
  game_storage_t &storage = game_get_storage();
  for (int i = 0; i < kNumCols; i++) {
    for (int j = 0; j < kNumRows; j++) {
      storage.cells[i][j] = random_grid();
    }
  }
}

void game_init() {
  // Note: If all data cell in game_get_storage() is 0, then it's new and should
  // be inited. Init any tasks here.
  game_storage_t &storage = game_get_storage();

  // check if the storage is empty
  bool empty = true;
  for (int i = 0; i < kNumCols; i++) {
    for (int j = 0; j < kNumRows; j++) {
      if (grid_score(&storage.cells[i][j]) != 0) {
        empty = false;
      }
    }
  }

  // if empty, generate random data
  if (empty) {
    init_game_storage();
  }

  // TODO: init other tasks
}

game_storage_t &game_get_storage() {
  return g_nv_storage.GetCurrentStorage().game_storage;
}

// Generate a new grid and replace existing grid if score is higher.
// Should be called periodically.
void __game_generate_random_and_update_column(int column) {
  grid_cell_t new_grid = random_grid();
  for (int i = 0; i < kNumRows; i++) {
    if (grid_score(&new_grid) >
        grid_score(&game_get_storage().cells[column][i])) {
      // TODO: sort column and replace the lowest score with the new grid
      return;
    }
  }
}

// Parse the received grid (either from other player or broadcaster) and call
// this function to check if the grid has higher score than the existing grid.
// If so, replace the existing grid.
void __game_receive_and_update_column(int column, void *event_data) {
  // TODO
}

void game_accept_data(int col, uint8_t *data) {
  // TODO: schedule task deal with accepting data
  //game_queue.push(col, data);
}
bool get_random_cell_data_for_ir_transmission(uint8_t *out_data, int *out_col) {
  return true;
}

}  // namespace game
}  // namespace hitcon
