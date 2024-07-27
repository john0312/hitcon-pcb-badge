#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdint.h>

/**
 * Definition of columns:
 * 0: self generated (unshared)
 * 1: self generated, shared with other players (send/receive)
 * 2: received from broadcasters scattered around the world
 * 3: (same as 2)
 * ...
 * 7: (same as 2)
 */

#define GAME_COLUMN_UNSHARED 0      // column index
#define GAME_COLUMN_SHARED_PLAYER 1 // column index
#define GAME_COLUMN_BROADCASTER1 2  // column index
#define GAME_COLUMN_BROADCASTER2 3  // column index
#define GAME_COLUMN_BROADCASTER3 4  // column index
#define GAME_COLUMN_BROADCASTER4 5  // column index
#define GAME_COLUMN_BROADCASTER5 6  // column index
#define GAME_COLUMN_BROADCASTER6 7  // column index

#define GAME_UPDATE_UNSHARED_PERIOD (10 * 60) // 10 minutes
#define GAME_UPDATE_SHARED_PERIOD (10 * 60)   // 10 minutes

#define GAME_NUM_COLUMNS 8
#define GAME_NUM_ROWS 16
#define GAME_DATA_SIZE 8

typedef int score_t;

typedef struct {
  uint8_t data[GAME_DATA_SIZE];
} grid_t;

typedef struct {
  grid_t data[GAME_NUM_COLUMNS][GAME_NUM_ROWS];
} storage_t;

// Initialize the game.
// If storage is NULL, this function will generate one.
void game_init(const storage_t *storage);

// Get the storage of the game. Can be used to save the game (persistent
// storage).
storage_t game_get_storage();

typedef void (*callback_no_arg_t)(void);
typedef void (*callback_event_data_t)(void *event_data);
void register_callbacks(void (*register_callback_no_arg)(
                            int period, callback_no_arg_t callback_no_arg),
                        void (*register_callback_event_data)(
                            callback_event_data_t callback_event_data));

// This is the data we received from peer.
void game_accept_data(int col, uint8_t* data);

// Retrieve the data at particular cell.
uint8_t* get_data_cell(int col, int row);

#endif // GAME_H
