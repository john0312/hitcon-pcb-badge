#ifdef HITCON_TEST_MODE

#include <Logic/Display/display.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>

#include "TetrisGame.h"

std::mutex game_mutex;
hitcon::tetris::TetrisGame game([]() { return static_cast<unsigned>(rand()); });

void print_buf_90(display_buf_t* buf_) {
  // will rotate 90 degree since tetris is portrait
  uint8_t buf[DISPLAY_WIDTH * DISPLAY_HEIGHT];
  display_buf_unpack(buf, buf_);
  uint8_t buf90[DISPLAY_WIDTH][DISPLAY_HEIGHT];
  for (int x = 0; x < DISPLAY_WIDTH; x++) {
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
      buf90[x][DISPLAY_HEIGHT - 1 - y] = buf[y * DISPLAY_WIDTH + x];
    }
  }
  for (int x = 0; x < DISPLAY_WIDTH; x++) {
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
      uint8_t ch = buf90[x][y];
      printf("%c%c", ch == 0 ? '.' : '0' + ch,
             y == DISPLAY_HEIGHT - 1 ? '\n' : ' ');
    }
  }
}

// Function to set terminal to raw mode
void setTerminalRawMode(bool enable) {
  static struct termios oldt, newt;
  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
}

// Background function to be run in a separate thread
void gameFunction() {
  auto now_ms = []() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  };
  auto prev_update = now_ms() - hitcon::tetris::FALL_PERIOD;
  game.game_start_playing();
  while (1) {
    auto now = now_ms();
    if (now - prev_update >= hitcon::tetris::FALL_PERIOD) {
      std::lock_guard<std::mutex> lock(game_mutex);
      game.game_fall_down_tetromino();
      prev_update = now;
      if (game.game_get_state() == hitcon::tetris::GAME_STATE_GAME_OVER) {
        break;
      }
    }

    display_buf_t buf[DISPLAY_WIDTH] = {0};
    {
      std::lock_guard<std::mutex> lock(game_mutex);
      game.game_draw_to_display(buf);
    }
    print_buf_90(buf);
    puts("");

    std::this_thread::sleep_for(
        std::chrono::milliseconds(hitcon::tetris::UPDATE_INTERVAL));
  }
}

void ioFunction() {
  while (1) {
    {
      std::lock_guard<std::mutex> lock(game_mutex);
      if (game.game_get_state() == hitcon::tetris::GAME_STATE_GAME_OVER) {
        break;
      }
    }

    if (std::fgetc(stdin) != EOF) {
      char ch = std::fgetc(stdin);
      switch (ch) {
        case 'a': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_on_input(hitcon::tetris::DIRECTION_LEFT);
          break;
        }

        case 'd': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_on_input(hitcon::tetris::DIRECTION_RIGHT);
          break;
        }

        case 's': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_on_input(hitcon::tetris::DIRECTION_DOWN);
          break;
        }

        case 'w': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_on_input(hitcon::tetris::DIRECTION_UP);
          break;
        }

        case 't': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_enemy_attack(2);
          break;
        }

        case 'f': {
          std::lock_guard<std::mutex> lock(game_mutex);
          game.game_on_input(hitcon::tetris::DIRECTION_FAST_DOWN);
          break;
        }

        default:
          break;
      }
    }
  }
}

int main() {
  setTerminalRawMode(true);
  atexit([]() { setTerminalRawMode(false); });

  std::thread gameThread(gameFunction);
  std::thread ioThread(ioFunction);

  gameThread.join();
  ioThread.join();

  return 0;
}

#endif
