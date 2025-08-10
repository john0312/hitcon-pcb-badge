#ifndef DISPLAY_H
#define DISPLAY_H

#include <Logic/Display/font.h>
#include <stdint.h>
#include <string.h>

// clang-format off
/**
 * The original design of display buffer is a 2D array of uint8_t.
 * This is the default buffer layout used in every API.
 *
 *    buf[0][0] buf[0][1] ... buf[0][DISPLAY_WIDTH - 1]
 *    buf[1][0] buf[1][1] ... buf[1][DISPLAY_WIDTH - 1]
 *    ...
 *    buf[DISPLAY_HEIGHT - 1][0] ... buf[DISPLAY_HEIGHT - 1][DISPLAY_WIDTH - 1]
 *
 *
 * However, we want to reduce the memory usage. We know that DISPLAY_HEIGHT
 * is fixed to 8, so we store each column in a display_buf_t (uint8_t).
 *
 *    (buf[0] & 1)   (buf[1] & 1)   ... (buf[DISPLAY_WIDTH - 1] & 1)
 *    (buf[0] & 2)   (buf[1] & 2)   ... (buf[DISPLAY_WIDTH - 1] & 2)
 *    (buf[0] & 4)   (buf[1] & 4)   ... (buf[DISPLAY_WIDTH - 1] & 4)
 *    ...
 *    (buf[0] & 128) (buf[1] & 128) ... (buf[DISPLAY_WIDTH - 1] & 128)
 */
// clang-format on

#define DISPLAY_HEIGHT 8  // fixed
#define DISPLAY_WIDTH 16  // adjustable, maybe 8, 12, 16

#define DISPLAY_MODE_BLANK 0
#define DISPLAY_MODE_FIXED 1
#define DISPLAY_MODE_SCROLL 2
#define DISPLAY_MODE_TEXT_EDITOR 3

#define DISPLAY_SCROLL_MAX_COLUMNS 170
#define DISPLAY_SCROLL_DEFAULT_SPEED 8

constexpr size_t kDisplayScrollMaxTextLen =
    DISPLAY_SCROLL_MAX_COLUMNS / CHAR_WIDTH - 1;

constexpr size_t kDisplayMaxNameLength = kDisplayScrollMaxTextLen - 5;
using display_buf_t = uint8_t;
static_assert(sizeof(display_buf_t) == DISPLAY_HEIGHT / 8,
              "The size of display_buf_t should be DISPLAY_HEIGHT / 8");

#define display_buf_set(buf, bit) \
  do {                            \
    buf |= (1 << (bit));          \
  } while (0)

#define display_buf_unset(buf, bit) \
  do {                              \
    buf &= ~(1 << (bit));           \
  } while (0)

#define display_buf_assign(buf, bit, val)           \
  do {                                              \
    buf = (buf & ~(1 << (bit))) | ((val) << (bit)); \
  } while (0)

#define display_buf_flip(buf, bit) \
  do {                             \
    buf ^= (1 << (bit));           \
  } while (0)

#define display_buf_get(buf, bit) (!!(buf & (1 << (bit))))

#define display_buf_render_char(buf, ch, x, y, max_x, max_y)          \
  do {                                                                \
    for (int _y = 0; _y < CHAR_HEIGHT && (y) + _y < (max_y); ++_y) {  \
      for (int _x = 0; _x < CHAR_WIDTH && (x) + _x < (max_x); ++_x) { \
        display_buf_assign(buf[(x) + _x], (y) + _y,                   \
                           rasterize_char_5x8((ch), _y, _x));         \
      }                                                               \
    }                                                                 \
  } while (0)

// Pack uint8_t buffer to display_buf_t buffer to save memory.
inline void display_buf_pack(display_buf_t *dst, const uint8_t *src) {
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      display_buf_assign(dst[x], y, src[y * DISPLAY_WIDTH + x]);
    }
  }
}

inline void display_buf_pack(display_buf_t *dst, const uint8_t *src,
                             int n_col) {
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < n_col; ++x) {
      display_buf_assign(dst[x], y, src[y * n_col + x]);
    }
  }
}

inline void display_buf_unpack(uint8_t *dst, const display_buf_t *src) {
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      dst[y * DISPLAY_WIDTH + x] = display_buf_get(src[x], y);
    }
  }
}

inline void display_buf_unpack(uint8_t *dst, const display_buf_t *src,
                               int n_col) {
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < n_col; ++x) {
      dst[y * n_col + x] = display_buf_get(src[x], y);
    }
  }
}

inline void display_buf_rotate_180(display_buf_t *buf) {
  display_buf_t tmp[DISPLAY_WIDTH];
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      display_buf_assign(tmp[DISPLAY_WIDTH - 1 - x], DISPLAY_HEIGHT - 1 - y,
                         display_buf_get(buf[x], y));
    }
  }
  memcpy(buf, tmp, sizeof(tmp));
}

void display_init();

// Get the frame of the display at the given frame.
// The size of `buf` should be DISPLAY_HEIGHT * DISPLAY_WIDTH
void display_get_frame(uint8_t *buf, int frame);

// The memory-efficient version of `display_get_frame`.
void display_get_frame_packed(display_buf_t *buf, int frame);

void display_set_mode_blank();

// size of `buf` should be DISPLAY_HEIGHT * DISPLAY_WIDTH
void display_set_mode_fixed(const uint8_t *buf);

// The memory-efficient version of `display_set_mode_fixed`.
void display_set_mode_fixed_packed(const display_buf_t *buf);

// size of `buf` should be DISPLAY_HEIGHT * `n_col`
// maximum `n_col` is DISPLAY_SCROLL_MAX_COLUMNS
// `speed` means how many frames to move one pixel
void display_set_mode_scroll(const uint8_t *buf, int n_col, int speed);
void display_set_mode_scroll(const uint8_t *buf, int n_col);

// The memory-efficient version of `display_set_mode_scroll`.
void display_set_mode_scroll_packed(const display_buf_t *buf, int n_col,
                                    int speed);
void display_set_mode_scroll_packed(const display_buf_t *buf, int n_col);

void display_set_mode_text(const char *text);

// Rasterize `text` to the underlying display buffer.
// If the text is too long, the output will be truncated.
void display_set_mode_scroll_text(const char *text,
                                  int speed = DISPLAY_SCROLL_DEFAULT_SPEED);

// Get the number of times the display has scrolled.
// Example use case: an app can use this to scroll the text only once
// and then stop.
// Returns -1 if the display is not in scroll mode.
int display_get_scroll_count();

enum DisplaySetModeState {
  SET_MODE_IDLE,
  SET_MODE_ST_RENDER,
  SET_MODE_ST_FINAL
};

void display_set_mode_internal_taskfunc(void *arg1, void *arg2);

namespace hitcon {
class TextEditorDisplay;
}

// Set the display to the text editor mode. The logic is outsourced to the
// `TextEditorDisplay` class.
void display_set_mode_editor(hitcon::TextEditorDisplay *editor);
extern int display_set_mode_orientation;

// Set display orientation
void display_set_orientation(int orientation);

#endif
