#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/**
 * The underlying buffer is a 2D array of uint8_t like this:
 *
 * buf[0][0] buf[0][1] ... buf[0][DISPLAY_WIDTH - 1]
 * buf[1][0] buf[1][1] ... buf[1][DISPLAY_WIDTH - 1]
 * ...
 * buf[DISPLAY_HEIGHT - 1][0] ... buf[DISPLAY_HEIGHT - 1][DISPLAY_WIDTH - 1]
 */

#define DISPLAY_HEIGHT 8 // fixed
#define DISPLAY_WIDTH 8  // adjustable, maybe 8, 12, 16

#define DISPLAY_MODE_BLANK 0
#define DISPLAY_MODE_FIXED 1
#define DISPLAY_MODE_SCROLL 2

#define DISPLAY_SCROLL_MAX_COLUMNS 48

void display_init();

// Get the frame of the display at the given frame.
// The size of `buf` should be DISPLAY_HEIGHT * DISPLAY_WIDTH
void display_get_frame(uint8_t *buf, int frame);

void display_set_mode_blank();

// size of `buf` should be DISPLAY_HEIGHT * DISPLAY_WIDTH
void display_set_mode_fixed(uint8_t *buf);

// size of `buf` should be DISPLAY_HEIGHT * `cols`
// maximum `cols` is DISPLAY_SCROLL_MAX_COLUMNS
// `speed` means how many frames to move one pixel
void display_set_mode_scroll(uint8_t *buf, int cols, int speed);

// TODO
// void display_set_mode_scroll_text(const char *text);

#endif
