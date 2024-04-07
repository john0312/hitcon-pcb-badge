#include "game.h"

static storage_t __storage;

grid_t random_grid() {
  // TODO
  return (grid_t){0};
}

score_t grid_score(const grid_t *grid) {
  // TODO
  return 0;
}

storage_t new_storage() {
  storage_t storage;
  for (int i = 0; i < GAME_NUM_COLUMNS; i++) {
    for (int j = 0; j < GAME_NUM_ROWS; j++) {
      for (int k = 0; k < GAME_DATA_SIZE; k++) {
        storage.data[i][j] = random_grid();
      }
    }
  }
  return storage;
}

void game_init(const storage_t *storage) {
  if (storage == NULL) {
    __storage = new_storage();
    return;
  }
  __storage = *storage;
}

storage_t game_get_storage() { return __storage; }

void __game_generate_random_and_update_column(int column) {
  grid_t new_grid = random_grid();
  for (int i = 0; i < GAME_NUM_ROWS; i++) {
    if (grid_score(&new_grid) > grid_score(&__storage.data[column][i])) {
      // TODO: sort column and replace the lowest score with the new grid
      return;
    }
  }
}

void game_update_unshared() {
  __game_generate_random_and_update_column(GAME_COLUMN_UNSHARED);
}

void game_update_shared_periodic() {
  __game_generate_random_and_update_column(GAME_COLUMN_SHARED);
}

void game_update_shared_event(void *event_data) {
  // TODO
}

void game_update_broadcast_event(void *event_data) {
  // TODO
}
