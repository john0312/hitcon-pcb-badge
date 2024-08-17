#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <Logic/Display/display.h>

namespace hitcon {
namespace tetris {

constexpr unsigned FALL_PERIOD = 1000;     // ms
constexpr unsigned UPDATE_INTERVAL = 200;  // ms
constexpr unsigned UPDATE_PRIORITY = 960;

constexpr int BOARD_WIDTH = 8;
constexpr int BOARD_HEIGHT = 16;
constexpr int TETROMINO_COUNT = 7;
static_assert(BOARD_WIDTH == DISPLAY_HEIGHT,
              "BOARD_WIDTH must be equal to DISPLAY_HEIGHT");
static_assert(BOARD_HEIGHT == DISPLAY_WIDTH,
              "BOARD_HEIGHT must be equal to DISPLAY_WIDTH");

enum TetrisGameState {
  GAME_STATE_WAITING,
  GAME_STATE_PLAYING,
  GAME_STATE_GAME_OVER,
};

enum TetrisDirection {
  DIRECTION_UP,
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
};

// clang-format off
/**
 * LSB <--> MSB
 *
 * I XXXX  J XXX   L XXX   O XX
 *           ..X     X..     XX
 *
 * S .XX   Z XX.   T XXX
 *   XX.     .XX     .X.
 */
const struct tetromino_t {
  int width;
  int height;
  // The rotated tetromino should be placed at (x + rotate_dx, y + rotate_dy), i.e. TETROMINO[0][1]
  // should be placed at (x + TETROMINO[0][0].rotate_dx, y + TETROMINO[0][0].rotate_dy).
  int rotate_dx;
  int rotate_dy;
  uint8_t data[4];
} TETROMINO[TETROMINO_COUNT][4] = {
  { // I
    {4, 1, 1, -1, {0b1111}},
    {1, 4, -2, 1, {0b1, 0b1, 0b1, 0b1}},
    {4, 1, 2, -2, {0b1111}},
    {1, 4, -1, 2, {0b1, 0b1, 0b1, 0b1}},
  },
  { // J
    {3, 2, 0, -1, {0b111, 0b100}},
    {2, 3, 0, 0, {0b10, 0b10, 0b11}},
    {3, 2, 1, 0, {0b001, 0b111}},
    {2, 3, -1, 1, {0b11, 0b1, 0b1}},
  },
  { // L
    {3, 2, 0, -1, {0b111, 0b1}},
    {2, 3, 0, 0, {0b11, 0b10, 0b10}},
    {3, 2, 1, 0, {0b100, 0b111}},
    {2, 3, -1, 1, {0b1, 0b1, 0b11}},
  },
  { // O
    {2, 2, 0, 0, {0b11, 0b11}},
    {2, 2, 0, 0, {0b11, 0b11}},
    {2, 2, 0, 0, {0b11, 0b11}},
    {2, 2, 0, 0, {0b11, 0b11}},
  },
  { // S
    {3, 2, 0, -1, {0b110, 0b011}},
    {2, 3, 0, 0, {0b01, 0b11, 0b10}},
    {3, 2, 1, 0, {0b110, 0b011}},
    {2, 3, -1, 1, {0b01, 0b11, 0b10}},
  },
  { // T
    {3, 2, 0, -1, {0b111, 0b010}},
    {2, 3, 0, 0, {0b10, 0b11, 0b10}},
    {3, 2, 1, 0, {0b010, 0b111}},
    {2, 3, -1, 1, {0b1, 0b11, 0b1}},
  },
  { // Z
    {3, 2, 0, -1, {0b011, 0b110}},
    {2, 3, 0, 0, {0b10, 0b11, 0b01}},
    {3, 2, 1, 0, {0b011, 0b110}},
    {2, 3, -1, 1, {0b10, 0b11, 0b1}},
  },
};
// clang-format on

/**
 * The Tetris game.
 * This class is only responsible for the game logic. To interact with the user
 * and the board, see TetrisApp.
 */
class TetrisGame {
 private:
  TetrisGameState state = GAME_STATE_WAITING;
  unsigned last_fall_time = 0;
  int current_tetromino;
  int current_x;
  int current_y;
  int current_rotation;
  static constexpr int CLEAR_LINE_SCORE = 10;
  int score = 0;

  // board[0] is the top row, board[1] is the second row, etc.
  // Each row is a byte, and each bit represents a cell.
  // The LSB is the leftmost cell.
  uint8_t board[BOARD_HEIGHT] = {0};
  static_assert(BOARD_WIDTH == sizeof(uint8_t) * 8,
                "TETRIS_BOARD_WIDTH must be 8");

  unsigned (*__rand)(void) = nullptr;
  void (*attack_enemy_callback)(int n_lines) = nullptr;

  inline unsigned rand() { return __rand ? __rand() : 0; }
  void clear_full_line();
  bool rotate_tetromino();
  bool place_tetromino(int x, int y);
  bool place_tetromino(int x, int y, int rotation);
  void unplace_tetromino(int x, int y);
  void fall_down_tetromino();
  bool generate_new_tetromino();
  inline void game_over() { state = GAME_STATE_GAME_OVER; }

 public:
  TetrisGame() = default;
  TetrisGame(unsigned (*rand)(void));

  void game_fall_down_tetromino();
  void game_on_input(TetrisDirection direction);
  void game_draw_to_display(display_buf_t *buf);
  inline void game_start_playing() { state = GAME_STATE_PLAYING; }
  inline void game_force_over() { game_over(); }
  inline TetrisGameState game_get_state() const { return state; };
  inline int game_get_score() const { return score; }

  // 2-player game, this function should be called when enemy attacks us.
  void game_enemy_attack(int n_lines);
  // 2-player game, the callback will be called when we attack enemy.
  void game_register_attack_enemy_callback(void (*callback)(int n_lines));
};

}  // namespace tetris
}  // namespace hitcon

#endif  // TETRIS_GAME_H
