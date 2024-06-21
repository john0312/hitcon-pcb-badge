#include "editor.h"
#include <Display/display.h>
#include <Display/font.h>
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

void TextEditorDisplay::draw(uint8_t *display_buf) const {
  // TODO
}

} // namespace hitcon
