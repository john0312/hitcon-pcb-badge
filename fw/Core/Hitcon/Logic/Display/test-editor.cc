#ifdef HITCON_TEST_MODE

#include <stdio.h>
#include <unistd.h>

#include "display.h"
#include "editor.h"

#define tick(editor, buf, frame)     \
  do {                               \
    display_get_frame(buf, frame++); \
    print_buf(frame, buf);           \
    usleep(100000);                  \
  } while (0)

void print_buf(int frame, uint8_t *buf) {
  printf("frame %d\n", frame);
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      uint8_t ch = buf[y * DISPLAY_WIDTH + x];
      printf("%c%c", ch == 0 ? '.' : '0' + ch,
             x == DISPLAY_WIDTH - 1 ? '\n' : ' ');
    }
  }
}

int main() {
  hitcon::TextEditorDisplay editor("HITCON2025");

  display_init();
  uint8_t buf[DISPLAY_HEIGHT * DISPLAY_WIDTH];
  int frame = 0;

  display_set_mode_editor(&editor);

  // test blinking cursor
  for (int i = 0; i < 30; i++) {
    tick(editor, buf, frame);
  }

  // test moving cursor
  for (int i = 0; i < 15; i++) {
    editor.move_cursor_left();
    for (int j = 0; j < 10; j++) {
      tick(editor, buf, frame);
    }
  }
  for (int i = 0; i < 15; i++) {
    editor.move_cursor_right();
    for (int j = 0; j < 10; j++) {
      tick(editor, buf, frame);
    }
  }

  // test changing characters
  for (int i = 0; i < 100; i++) {
    editor.incr_current_char();
    for (int j = 0; j < 2; j++) {
      tick(editor, buf, frame);
    }
  }

  return 0;
}

#endif  // HITCON_TEST_MODE
