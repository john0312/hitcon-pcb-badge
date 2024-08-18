#ifndef EDITOR_H
#define EDITOR_H

#include <Logic/Display/display.h>
#include <Logic/Display/font.h>

#include <cstdint>

namespace hitcon {

class TextEditorDisplay {
 public:
  constexpr static int MAX_TEXT_LENGTH = kDisplayScrollMaxTextLen;
  constexpr static int BLINK_CURSOR_PERIOD = 250;

  char text[MAX_TEXT_LENGTH + 1] = {0};
  int cursor;

  TextEditorDisplay();
  TextEditorDisplay(const char *init_text);

  void move_cursor_left();
  void move_cursor_right();
  void incr_current_char();
  void decr_current_char();
  void set_current_char(char c);
  void backspace();
  void insert();

  void draw(uint8_t *buf, int frame) const;
  void draw_packed(display_buf_t *display_buf, int frame) const;
};

}  // namespace hitcon

#endif  // EDITOR_H
