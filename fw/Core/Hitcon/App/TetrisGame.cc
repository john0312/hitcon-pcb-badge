#include "TetrisGame.h"

namespace hitcon {
namespace tetris {

TetrisGame::TetrisGame(unsigned (*rand)(void)) : __rand(rand) {
  generate_new_tetromino();
}

void TetrisGame::clear_full_line() {
  int from = BOARD_HEIGHT - 1;
  int to = BOARD_HEIGHT - 1;
  int cleared_lines = 0;
  for (; from >= 0; from--) {
    if (board[from] == 0b11111111u) {
      cleared_lines++;
    } else {
      board[to] = board[from];
      if (to != from) {
        board[from] = 0;
      }
      to--;
    }
  }
  for (; to >= 0; to--) {
    board[to] = 0;
  }

  score += cleared_lines * CLEAR_LINE_SCORE;
  if (attack_enemy_callback && cleared_lines > 0) {
    attack_enemy_callback(cleared_lines);
  }
}

// If success, return true and update current_x, current_y, current_rotation.
bool TetrisGame::rotate_tetromino() {
  int new_rotation = (current_rotation + 1) % 4;
  const auto &t_old = TETROMINO[current_tetromino][current_rotation];
  int new_x = current_x + t_old.rotate_dx;
  int new_y = current_y + t_old.rotate_dy;
  if (!place_tetromino(new_x, new_y, new_rotation)) {
    return false;
  }
  return true;
}

// If success, return true and update current_x, current_y, current_rotation.
bool TetrisGame::place_tetromino(int x, int y) {
  return place_tetromino(x, y, current_rotation);
}

// If success, return true and update current_x, current_y, current_rotation.
bool TetrisGame::place_tetromino(int x, int y, int rotation) {
  const auto &t = TETROMINO[current_tetromino][rotation];
  // check if the tetromino is out of the board
  if (!(0 <= x && x + t.width <= BOARD_WIDTH)) {
    return false;
  }
  if (!(0 <= y && y + t.height <= BOARD_HEIGHT)) {
    return false;
  }
  // check if the position is occupied
  for (int i = 0; i < t.height; i++) {
    if (!!(board[y + i] & (t.data[i] << x))) {
      return false;
    }
  }
  // place the tetromino
  for (int i = 0; i < t.height; i++) {
    board[y + i] |= (t.data[i] << x);
  }
  current_x = x;
  current_y = y;
  current_rotation = rotation;
  return true;
}

void TetrisGame::unplace_tetromino(int x, int y) {
  const auto &t = TETROMINO[current_tetromino][current_rotation];
  for (int i = 0; i < t.height; i++) {
    board[y + i] &= ~(t.data[i] << x);
  }
}

// If success, update current_y.
// If fail, determine if the game is over or generate a new tetromino.
void TetrisGame::fall_down_tetromino() {
  unplace_tetromino(current_x, current_y);
  if (place_tetromino(current_x, current_y + 1)) {
    return;
  }

  // if fail, check if game is over
  if (current_y == 0) {
    game_over();
    return;
  }

  // fix the tetromino at current position
  place_tetromino(current_x, current_y);
  clear_full_line();

  // generate a new tetromino
  if (!generate_new_tetromino()) {
    game_over();
  }
}

bool TetrisGame::generate_new_tetromino() {
  int new_tetromino;
  do {
    if (__rand == nullptr) {
      new_tetromino ^= 1;
    } else {
      new_tetromino = rand() % TETROMINO_COUNT;
    }
  } while (new_tetromino == current_tetromino);
  current_tetromino = new_tetromino;
  const auto &t = TETROMINO[current_tetromino][0];
  current_x = BOARD_WIDTH / 2 - t.width / 2;
  current_y = 0;
  current_rotation = 0;
  return place_tetromino(current_x, current_y);
}

void TetrisGame::game_fall_down_tetromino() {
  if (state == GAME_STATE_PLAYING) {
    fall_down_tetromino();
  }
}

void TetrisGame::game_on_input(TetrisDirection direction) {
  if (state == GAME_STATE_PLAYING) {
    switch (direction) {
      case DIRECTION_LEFT:
        unplace_tetromino(current_x, current_y);
        if (!place_tetromino(current_x - 1, current_y)) {
          place_tetromino(current_x, current_y);
        }
        break;

      case DIRECTION_RIGHT:
        unplace_tetromino(current_x, current_y);
        if (!place_tetromino(current_x + 1, current_y)) {
          place_tetromino(current_x, current_y);
        }
        break;

      case DIRECTION_DOWN:
        fall_down_tetromino();
        break;

      case DIRECTION_UP:
        unplace_tetromino(current_x, current_y);
        if (!rotate_tetromino()) {
          place_tetromino(current_x, current_y);
        }
        break;

      case DIRECTION_FAST_DOWN:
        unplace_tetromino(current_x, current_y);
        while (place_tetromino(current_x, current_y + 1)) {
          unplace_tetromino(current_x, current_y);
        }
        place_tetromino(current_x, current_y);
        fall_down_tetromino();
        break;

      default:
        break;
    }
  }
}

inline uint8_t reverse_bit(uint8_t x) {
  x = ((x & 0b10101010u) >> 1) | ((x & 0b01010101u) << 1);
  x = ((x & 0b11001100u) >> 2) | ((x & 0b00110011u) << 2);
  x = ((x & 0b11110000u) >> 4) | ((x & 0b00001111u) << 4);
  return x;
}

void TetrisGame::game_draw_to_display(display_buf_t *buf) {
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    buf[y] = reverse_bit(board[y]);
  }
}

void TetrisGame::game_enemy_attack(int n_lines) {
  // move the current board upward by n_lines
  unplace_tetromino(current_x, current_y);
  for (int i = 0; i < BOARD_HEIGHT - n_lines; i++) {
    board[i] = board[i + n_lines];
  }
  place_tetromino(current_x, current_y);

  // add n_lines lines of garbage to the bottom of the board
  // note that the garbage lines are full line with 1 bit unset
  for (int i = BOARD_HEIGHT - n_lines; i < BOARD_HEIGHT; i++) {
    board[i] = (0b11111111u ^ (1u << (rand() % BOARD_WIDTH)));
  }
}

void TetrisGame::game_register_attack_enemy_callback(
    void (*callback)(int n_lines)) {
  attack_enemy_callback = callback;
}

}  // namespace tetris
}  // namespace hitcon
