#ifndef LOGIC_GAME_DOT_H_
#define LOGIC_GAME_DOT_H_

#include <stddef.h>
#include <stdint.h>

#include <Logic/game_param.h>

namespace hitcon {
namespace game {

// Initialize the game.
// If storage is NULL, this function will generate one.
void game_init(const game_storage_t *storage);

// Get the storage of the game. Can be used to save the game (persistent
// storage).
game_storage_t& game_get_storage();

// This is the data we received from peer.
void game_accept_data(int col, uint8_t* data);

// Retrieve the data at particular cell.
uint8_t* get_data_cell(int col, int row);

// Get a random data cell for sending over IR. The data is written to out_data, while the column is written to out_col.
bool get_random_cell_data_for_ir_transmission(uint8_t* out_data, int* out_col);

}  // namespace game
}  // namespace hitcon

#endif // LOGIC_GAME_DOT_H_
