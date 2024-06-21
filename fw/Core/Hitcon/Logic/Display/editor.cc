#include "editor.h"
#include <Logic/Display/display.h>
#include <Logic/Display/font.h>
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

void TextEditorDisplay::draw(uint8_t *display_buf, int frame) const {
  constexpr int max_text_num = DISPLAY_WIDTH / CHAR_WIDTH;
  char text_to_display[max_text_num + 1] = {0};
  int cursor_index_in_display;

  if (cursor <= max_text_num) {
    // case: cursor <= maximum displayable text length
    // Just display all characters.
    strncpy(text_to_display, text, max_text_num);
    cursor_index_in_display = cursor;
  } else {
    // case: cursor > maximum displayable text length
    // Display the last `max_text_num` - 1 characters since we need to display
    // the cursor.
    strncpy(text_to_display, text + cursor - max_text_num + 1, max_text_num);
    cursor_index_in_display = max_text_num - 1;
  }

  // render the text to the 2D buffer
  char buf_2d[DISPLAY_HEIGHT][DISPLAY_WIDTH] = {0};
  for (int i = 0; i < strlen(text_to_display); ++i) {
    render_char(buf_2d, text_to_display[i], i * CHAR_WIDTH, 0, DISPLAY_WIDTH,
                DISPLAY_HEIGHT);
  }

  // blink the cursor
  if (frame % BLINK_CURSOR_PERIOD < BLINK_CURSOR_PERIOD / 2) {
    for (int x = 0; x < CHAR_WIDTH; ++x) {
      buf_2d[CHAR_HEIGHT - 1][cursor_index_in_display * CHAR_WIDTH + x] ^= 1;
    }
  }

  // flatten the 2D buffer to 1D buffer
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      display_buf[y * DISPLAY_WIDTH + x] = buf_2d[y][x];
    }
  }
}

} // namespace hitcon
