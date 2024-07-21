#ifdef HITCON_TEST_MODE

#include <stdio.h>
#include <unistd.h>

#include "display.h"

void print_buf(int frame, uint8_t *buf) {
  printf("frame %d\n", frame);
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      uint8_t ch = !!(buf[x] & (1 << y));
      printf("%c%c", ch == 0 ? '.' : '0' + ch,
             x == DISPLAY_WIDTH - 1 ? '\n' : ' ');
    }
  }
}

int main() {
  display_init();
  uint8_t buf[DISPLAY_WIDTH];
  int frame = 0;

  {
    puts("[blank]");
    display_set_mode_blank();
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    puts("");
  }

  {
    puts("[fixed]");
    uint8_t buf_fixed[DISPLAY_HEIGHT * DISPLAY_WIDTH] = {
        // clang-format off
        1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
        0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
        // clang-format on
    };
    uint8_t buf_fixed_compressed[DISPLAY_WIDTH];
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
      for (int j = 0; j < DISPLAY_WIDTH; j++) {
        if (buf_fixed[i * DISPLAY_WIDTH + j])
          DISPLAY_SET(buf_fixed_compressed[j], i);
        else
          DISPLAY_UNSET(buf_fixed_compressed[j], i);
      }
    }

    display_set_mode_fixed(buf_fixed_compressed);
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    display_get_frame(buf, frame++);
    print_buf(frame, buf);
    puts("");
  }

  {
    puts("[scroll]");
    constexpr uint8_t buf_scroll[] = {
        // clang-format off
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        // clang-format on
    };
    constexpr int scroll_n_col =
        sizeof(buf_scroll) / (sizeof(uint8_t) / sizeof(char));
    uint8_t buf_scroll_compressed[scroll_n_col];
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
      for (int j = 0; j < scroll_n_col; j++) {
        if (buf_scroll[i * scroll_n_col + j])
          DISPLAY_SET(buf_scroll_compressed[j], i);
        else
          DISPLAY_UNSET(buf_scroll_compressed[j], i);
      }
    }

    int speed = 3;
    display_set_mode_scroll(buf_scroll_compressed, scroll_n_col, speed);
    for (int i = 0; i < 100; i++) {
      display_get_frame(buf, frame++);
      print_buf(frame, buf);
      usleep(1000000 / 10 / speed);
    }
    puts("");
  }

  {
    puts("[scroll text]");
    int speed = 3;
    display_set_mode_scroll_text("Hello, world!", speed);
    for (int i = 0; i < 500; i++) {
      display_get_frame(buf, frame++);
      print_buf(frame, buf);
      usleep(1000000 / 10 / speed);
    }
    puts("");
  }

  return 0;
}

#endif
