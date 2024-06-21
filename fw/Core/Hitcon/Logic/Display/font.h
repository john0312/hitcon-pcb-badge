#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
#define PRINTABLE_START 32
#define PRINTABLE_END 127

#define rasterize_char_5x8(ch, row, col)                                       \
  !!(console_font_5x8[ch][row] & (1 << (7 - (col))))

extern unsigned char console_font_5x8[][8];
