
#include <Logic/GameLogic.h>
#include <Logic/NvStorage.h>
#include <Logic/RandomPool.h>

#include <cstring>

namespace hitcon {
namespace game {

GameLogic gameLogic;

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
  // if (sha3_Init256(&priv, 256) != SHA3_RETURN_OK) return 0;
  // TODO: update with col key
  // sha3_UpdateWord();
  // for (int i = 0; i < kDataSize; i++) sha3_UpdateWord(priv, grid->data[i]);

  // TODO: return what??
  // return q22_ln(priv);
}

GameLogic::GameLogic() {}

void GameLogic::InitGameStorage() {
  for (int i = 0; i < kNumCols; i++) {
    for (int j = 0; j < kNumRows; j++) {
      storage_->cells[i][j] = random_grid();
    }
  }
}

void GameLogic::Init(game_storage_t *storage) {
  storage_ = storage;
  // check if the storage is empty
  bool empty = true;
  for (int i = 0; i < kNumCols; i++) {
    for (int j = 0; j < kNumRows; j++) {
      if (grid_score(&storage->cells[i][j]) != 0) {
        empty = false;
      }
    }
  }

  // if empty, generate random data
  if (empty) {
    InitGameStorage();
  }

  // TODO: init other tasks
}

// Parse the received grid (either from other player or broadcaster) and call
// this function to check if the grid has higher score than the existing grid.
// If so, replace the existing grid.
void __game_receive_and_update_column(int column, void *event_data) {
  // TODO
}

bool GameLogic::AcceptData(int col, uint8_t *data) {
  // TODO: schedule task deal with accepting data
  // game_queue.push(col, data);
}

bool GameLogic::GetRandomDataForIrTransmission(uint8_t *out_data,
                                               int *out_col) {
  return true;
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

}  // namespace game
}  // namespace hitcon
