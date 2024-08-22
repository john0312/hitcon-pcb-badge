#ifdef HITCON_TEST_MODE

#include <Logic/Display/display.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>

#include "BouncingDVDApp.h"

std::mutex app_mutex;
hitcon::app::bouncing_dvd::BouncingDVD app([]() {
  return static_cast<unsigned>(rand());
});

void print_buf(display_buf_t *buf) {
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      uint8_t ch = display_buf_get(buf[x], y);
      printf("%c%c", ch == 0 ? '.' : '0' + ch,
             x == DISPLAY_WIDTH - 1 ? '\n' : ' ');
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
  while (1) {
    {
      std::lock_guard<std::mutex> lock(app_mutex);
      int now = now_ms();
      app.update(now);
    }

    display_buf_t buf[DISPLAY_WIDTH] = {0};
    {
      std::lock_guard<std::mutex> lock(app_mutex);
      app.draw(buf);
    }
    print_buf(buf);
    puts("");

    std::this_thread::sleep_for(
        std::chrono::milliseconds(hitcon::app::bouncing_dvd::UPDATE_INTERVAL));
  }
}

void ioFunction() {
  while (1) {
    char ch = std::fgetc(stdin);
    switch (ch) {
      case 'w':  // up arrow
      case 'i':  // "i"ncrease
      case 'u':  // "u"p
        app.inc_move_speed();
        break;

      case 's':  // down arrow
      case 'd':  // "d"ecrease or "d"own
        app.dec_move_speed();
        break;

      default:
        break;
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
