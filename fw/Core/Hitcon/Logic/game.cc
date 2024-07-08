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

// Generate a new grid and replace existing grid if score is higher.
// Should be called periodically.
void __game_generate_random_and_update_column(int column) {
  grid_t new_grid = random_grid();
  for (int i = 0; i < GAME_NUM_ROWS; i++) {
    if (grid_score(&new_grid) > grid_score(&__storage.data[column][i])) {
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

#define FUNC_GENERATE_RANDOM_AND_UPDATE_COLUMN(name, column) \
  void name() { __game_generate_random_and_update_column(column); }
#define FUNC_RECEIVE_AND_UPDATE_COLUMN(name, column)      \
  void name(void *event_data) {                           \
    __game_receive_and_update_column(column, event_data); \
  }

FUNC_GENERATE_RANDOM_AND_UPDATE_COLUMN(game_update_unshared,
                                       GAME_COLUMN_UNSHARED)
FUNC_GENERATE_RANDOM_AND_UPDATE_COLUMN(game_update_shared_periodic,
                                       GAME_COLUMN_SHARED_PLAYER)

FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_shared_event,
                               GAME_COLUMN_SHARED_PLAYER)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast1_event,
                               GAME_COLUMN_BROADCASTER1)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast2_event,
                               GAME_COLUMN_BROADCASTER2)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast3_event,
                               GAME_COLUMN_BROADCASTER3)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast4_event,
                               GAME_COLUMN_BROADCASTER4)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast5_event,
                               GAME_COLUMN_BROADCASTER5)
FUNC_RECEIVE_AND_UPDATE_COLUMN(game_update_broadcast6_event,
                               GAME_COLUMN_BROADCASTER6)

void register_callbacks(void (*register_callback_no_arg)(
                            int period, callback_no_arg_t callback_no_arg),
                        void (*register_callback_event_data)(
                            callback_event_data_t callback_event_data)) {
  register_callback_no_arg(GAME_UPDATE_UNSHARED_PERIOD, game_update_unshared);
  register_callback_no_arg(GAME_UPDATE_SHARED_PERIOD,
                           game_update_shared_periodic);

  register_callback_event_data(game_update_shared_event);
  register_callback_event_data(game_update_broadcast1_event);
  register_callback_event_data(game_update_broadcast2_event);
  register_callback_event_data(game_update_broadcast3_event);
  register_callback_event_data(game_update_broadcast4_event);
  register_callback_event_data(game_update_broadcast5_event);
  register_callback_event_data(game_update_broadcast6_event);
}
