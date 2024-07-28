#include "game.h"

#include <Logic/NvStorage.h>

namespace hitcon {
namespace game {

// Implements the ln() with approximate polynomial.
// The output is in Q9.22 fixed point number, while the input is in
// uint32_t integer format.
int32_t q22_ln(uint32_t) {
  // TODO
}

game_cache_t game_cache;

grid_cell_t random_grid() {
  // TODO
  return (grid_cell_t){0};
}

score_t grid_score(const grid_cell_t *grid) {
  // TODO
  // Note: This probably has to be reimplemented because it spans multiple
  // tasks.
  return 0;
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

}  // namespace game
}  // namespace hitcon
