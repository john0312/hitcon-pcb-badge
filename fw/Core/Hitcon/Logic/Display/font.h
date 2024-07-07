#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define PRINTABLE_START 32
#define PRINTABLE_END 127

#define rasterize_char_5x8(ch, row, col)                                       \
  !!(console_font_5x8[size_t(ch)][row] & (1 << (7 - (col))))

#define render_char(buf, ch, x, y, max_x, max_y)                               \
  do {                                                                         \
    for (int _y = 0; _y < CHAR_HEIGHT && y + _y < max_y; ++_y) {               \
      for (int _x = 0; _x < CHAR_WIDTH && x + _x < max_x; ++_x) {              \
        buf[y + _y][x + _x] = rasterize_char_5x8(ch, _y, _x);                  \
      }                                                                        \
    }                                                                          \
  } while (0)

extern unsigned char console_font_5x8[][8];
