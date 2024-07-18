#include "editor.h"

#include <Logic/Display/display.h>
#include <Logic/Display/font.h>

#include <algorithm>
#include <cstring>

namespace hitcon {

// If text's length is less than MAX_TEXT_LENGTH, return the index where next
// character should be inserted. Otherwise, return MAX_TEXT_LENGTH - 1.
inline size_t get_last_cursor(const char *text) {
  size_t len = strlen(text);
  return (len < TextEditorDisplay::MAX_TEXT_LENGTH)
             ? len
             : TextEditorDisplay::MAX_TEXT_LENGTH - 1;
}

TextEditorDisplay::TextEditorDisplay() : cursor(0) {
  memset(text, 0, sizeof(text));
}

// Initialize the text editor with the given text and set the cursor to the end
// of the text.
TextEditorDisplay::TextEditorDisplay(const char *init_text) {
  strncpy(text, init_text, sizeof(text) - 1);
  cursor = get_last_cursor(text);
}

void TextEditorDisplay::move_cursor_left() {
  cursor = (cursor == 0) ? get_last_cursor(text) : cursor - 1;
}

void TextEditorDisplay::move_cursor_right() {
  cursor = (cursor == get_last_cursor(text)) ? 0 : cursor + 1;
}

void TextEditorDisplay::incr_current_char() {
  switch (text[cursor]) {
    case 0:
      text[cursor] = PRINTABLE_START;
      break;
    case PRINTABLE_END - 1:
      text[cursor] = 0;
      break;
    default:
      text[cursor]++;
  }
}

void TextEditorDisplay::decr_current_char() {
  switch (text[cursor]) {
    case 0:
      text[cursor] = PRINTABLE_END - 1;
      break;
    case PRINTABLE_START:
      text[cursor] = 0;
      break;
    default:
      text[cursor]--;
  }
}

void TextEditorDisplay::set_current_char(char c) {
  if (PRINTABLE_START <= c && c < PRINTABLE_END) {
    text[cursor] = c;
  }
}

void TextEditorDisplay::draw(uint8_t *buf, int frame) const {
  display_buf_t display_buf[DISPLAY_WIDTH];
  draw_packed(display_buf, frame);
  display_buf_unpack(buf, display_buf);
}

void TextEditorDisplay::draw_packed(display_buf_t *display_buf,
                                    int frame) const {
  /**
   * max_text_num is the maximum number of characters that can be
   * fully-displayed. However, if there is any character after the cursor, it
   * needs to be displayed. Therefore, we need to display max_text_num + 1
   * characters but limit the cursor_index_in_display to max_text_num - 1.
   */

  constexpr int max_text_num = DISPLAY_WIDTH / CHAR_WIDTH;
  static_assert(max_text_num >= 2, "max_text_num must be at least 2");
  char text_to_display[max_text_num + 2] = {0};
  int cursor_index_in_display;

  cursor_index_in_display = std::min(cursor, max_text_num - 1);
  strncpy(text_to_display, text + cursor - cursor_index_in_display,
          max_text_num + 1);

  // render the text to display_buf_t
  memset(display_buf, 0, DISPLAY_WIDTH);
  for (int i = 0; i < strlen(text_to_display); ++i) {
    display_buf_render_char(display_buf, text_to_display[i], i * CHAR_WIDTH, 0,
                            DISPLAY_WIDTH, DISPLAY_HEIGHT);
  }

  // blink the cursor
  if (frame % BLINK_CURSOR_PERIOD < BLINK_CURSOR_PERIOD / 2) {
    for (int x = 0; x < CHAR_WIDTH - 1; ++x) {
      display_buf_flip(display_buf[cursor_index_in_display * CHAR_WIDTH + x],
                       CHAR_HEIGHT - 1);
    }
  }
}

}  // namespace hitcon
