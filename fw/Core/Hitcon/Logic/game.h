#ifndef LOGIC_GAME_DOT_H_
#define LOGIC_GAME_DOT_H_

#include <stddef.h>
#include <stdint.h>
#include <utility>

#include <Logic/GameParam.h>

namespace hitcon {
namespace game {

// Initialize the game.
// If storage is NULL, this function will generate one.
void game_init(const game_storage_t *storage);

// Get the storage of the game. Can be used to save the game (persistent
// storage).
game_storage_t& game_get_storage();

// Set the col prefix for col to out.
// Column prefix is "HITCON" + uint16_t(col)
void game_get_col_prefix(uint8_t* out, int col);

// This is the data we received from peer.
void game_accept_data(int col, uint8_t* data);

// Retrieve the data at particular cell.
uint8_t* get_data_cell(int col, int row);

// Get a random data cell for sending over IR. The data is written to out_data, while the column is written to out_col.
bool get_random_cell_data_for_ir_transmission(uint8_t* out_data, int* out_col);

struct GameQueue {
  std::pair<int, grid_cell_t> datas[kQueueSize];
  // [start, end)
  uint8_t queue_start = 0;
  uint8_t queue_end = 0;
  uint8_t size() {
    return (queue_end - queue_start) > 0 ?
    (queue_end - queue_start) :
    (queue_end - queue_start + kQueueSize);
  }
  bool push(int col, grid_cell_t data) {
    if (size() == 0) return false;
    datas[queue_end++] = std::make_pair(col, data);
    if(queue_end == kQueueSize)
      queue_end = 0;
    return true;
  }
  std::pair<int, grid_cell_t>& top() {
    return datas[queue_start];
  }

  void pop() {
    if (size() == 0) return;
    queue_start--;
    if (queue_start < 0)
      queue_start += kQueueSize;
  }
};

extern GameQueue game_queue;

}  // namespace game
}  // namespace hitcon

#endif // LOGIC_GAME_DOT_H_
