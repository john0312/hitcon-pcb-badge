#include "display.h"

#include <Logic/Display/editor.h>
#include <Logic/Display/font.h>
#include <Service/Sched/Checks.h>
#include <Service/Sched/Scheduler.h>
#include <string.h>

using hitcon::service::sched::my_assert;
using hitcon::service::sched::scheduler;
using hitcon::service::sched::task_callback_t;

static DisplaySetModeState display_set_mode_state = SET_MODE_IDLE;
static hitcon::service::sched::Task display_set_mode_internal_task(
    620, (task_callback_t)&display_set_mode_internal_taskfunc, nullptr);
static bool display_set_mode_internal_task_queued = false;

static char display_set_mode_internal_text_buffer[kDisplayScrollMaxTextLen + 1];
static display_buf_t
    display_set_mode_internal_scroll_buffer[DISPLAY_SCROLL_MAX_COLUMNS];
static size_t display_set_mode_internal_render_idx;
static int display_set_mode_speed;
int display_set_mode_orientation = 0;

static display_buf_t __display_buf[DISPLAY_WIDTH];

static int display_mode;
// will be updated when display_get_frame is called
static int display_current_frame;
static hitcon::TextEditorDisplay *text_editor_display;

// TODO: use union to save memory if we want to store other info for other modes
struct {
  display_buf_t buf[DISPLAY_SCROLL_MAX_COLUMNS];
  int first_frame;
  int n_col;
  int speed;
} display_scroll_info;

void get_scroll_frame_packed(display_buf_t *buf, int frame) {
  /**
   * The content will scroll from right to left, and the first frame of the
   * scrolling is an empty screen.
   *
   * In the following illustration:
   * s = speed
   * T = (DISPLAY_WIDTH + n_col + 1) * speed
   * (which is the period of the scrolling)
   *
   *                      +------------------------------------+
   * scroll buffer        |                                    |
   *                      +------------------------------------+
   * frame = [0, s)  +---+
   * display buffer  |   |
   *                 +---+
   * frame = [s, 2s)  +---+
   * display buffer   |   |
   *                  +---+
   * frame = [2s, 3s)  +---+
   * display buffer    |   |
   *                   +---+
   * ...
   * frame = [T - 3s, T - 2s)                                 +---+
   * display buffer                                           |   |
   *                                                          +---+
   * frame = [T - 2s, T - s)                                   +---+
   * display buffer                                            |   |
   *                                                           +---+
   * frame = [T - s, T)                                         +---+
   * display buffer                                             |   |
   *                                                            +---+
   */

  int total_width = DISPLAY_WIDTH + display_scroll_info.n_col + 1;
  int period = total_width * display_scroll_info.speed;
  int x_at_frame0 = -DISPLAY_WIDTH;
  int current_x = x_at_frame0 + (frame - display_scroll_info.first_frame) %
                                    period / display_scroll_info.speed;

  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      int inside_window =
          (0 <= current_x + x && current_x + x < display_scroll_info.n_col);
      display_buf_assign(
          buf[x], y,
          inside_window
              ? display_buf_get(display_scroll_info.buf[current_x + x], y)
              : 0);
    }
  }
}

void display_init() {
  display_mode = DISPLAY_MODE_BLANK;
  memset(__display_buf, 0, sizeof(__display_buf));
}

void display_get_frame(uint8_t *buf, int frame) {
  display_buf_t display_buf[DISPLAY_WIDTH];
  display_get_frame_packed(display_buf, frame);
  display_buf_unpack(buf, display_buf);
}

void display_get_frame_packed(display_buf_t *buf, int frame) {
  switch (display_mode) {
    case DISPLAY_MODE_BLANK:
      memset(buf, 0, sizeof(__display_buf));
      break;

    case DISPLAY_MODE_FIXED:
      memcpy(buf, __display_buf, sizeof(__display_buf));
      break;

    case DISPLAY_MODE_SCROLL:
      get_scroll_frame_packed(buf, frame);
      break;

    case DISPLAY_MODE_TEXT_EDITOR:
      text_editor_display->draw_packed(buf, frame);
      break;
  }

  display_current_frame = frame;
}

void display_set_mode_blank() {
  display_mode = DISPLAY_MODE_BLANK;
  memset(__display_buf, 0, sizeof(__display_buf));
}

void display_set_mode_fixed(const uint8_t *buf) {
  display_buf_t display_buf[DISPLAY_WIDTH];
  display_buf_pack(display_buf, buf);
  display_set_mode_fixed_packed(display_buf);
  display_set_mode_state = SET_MODE_IDLE;
}

void display_set_mode_fixed_packed(const display_buf_t *buf) {
  display_mode = DISPLAY_MODE_FIXED;
  memcpy(__display_buf, buf, sizeof(__display_buf));
}

void display_set_mode_scroll(const uint8_t *buf, int n_col, int speed) {
  display_buf_t display_buf[DISPLAY_SCROLL_MAX_COLUMNS];
  display_buf_pack(display_buf, buf, n_col);
  display_set_mode_scroll_packed(display_buf, n_col, speed);
}

void display_set_mode_scroll(const uint8_t *buf, int n_col) {
  display_set_mode_scroll(buf, n_col, DISPLAY_SCROLL_DEFAULT_SPEED);
}

void display_set_mode_scroll_packed(const display_buf_t *buf, int n_col,
                                    int speed) {
  display_mode = DISPLAY_MODE_SCROLL;
  display_scroll_info.first_frame = display_current_frame;
  display_scroll_info.n_col = n_col;
  display_scroll_info.speed = speed;
  memcpy(display_scroll_info.buf, buf, n_col);
}

void display_set_mode_scroll_packed(const display_buf_t *buf, int n_col) {
  display_set_mode_scroll_packed(buf, n_col, DISPLAY_SCROLL_DEFAULT_SPEED);
}

void display_set_mode_scroll_text(const char *text, int speed) {
  int len = strlen(text);
  if (len >= kDisplayScrollMaxTextLen) {
    len = kDisplayScrollMaxTextLen;
  }
  memset(display_set_mode_internal_text_buffer, 0,
         sizeof(display_set_mode_internal_text_buffer));
  memcpy(display_set_mode_internal_text_buffer, text, len);

  display_set_mode_state = SET_MODE_ST_RENDER;
  display_set_mode_internal_render_idx = 0;
  display_set_mode_speed = speed;

  if (!display_set_mode_internal_task_queued) {
    scheduler.Queue(&display_set_mode_internal_task, nullptr);
    display_set_mode_internal_task_queued = true;
  }
}

void display_set_mode_internal_taskfunc(void *arg1, void *arg2) {
  display_set_mode_internal_task_queued = false;
  switch (display_set_mode_state) {
    case SET_MODE_IDLE:
      return;
    case SET_MODE_ST_RENDER: {
      for (int i = 0;
           i < 4 && display_set_mode_internal_render_idx * CHAR_WIDTH <
                        DISPLAY_SCROLL_MAX_COLUMNS;
           i++, display_set_mode_internal_render_idx++) {
        display_buf_render_char(
            display_set_mode_internal_scroll_buffer,
            display_set_mode_internal_text_buffer
                [display_set_mode_internal_render_idx],
            display_set_mode_internal_render_idx * CHAR_WIDTH, 0,
            DISPLAY_SCROLL_MAX_COLUMNS, DISPLAY_HEIGHT);
      }
      if (display_set_mode_internal_render_idx * CHAR_WIDTH >=
          DISPLAY_SCROLL_MAX_COLUMNS) {
        display_set_mode_state = SET_MODE_ST_FINAL;
      }
      if (!display_set_mode_internal_task_queued) {
        scheduler.Queue(&display_set_mode_internal_task, nullptr);
        display_set_mode_internal_task_queued = true;
      }
      return;
    }
    case SET_MODE_ST_FINAL: {
      int len = strlen(display_set_mode_internal_text_buffer);
      if (len >= kDisplayScrollMaxTextLen) {
        len = kDisplayScrollMaxTextLen;
      }
      int n_col = (len * CHAR_WIDTH > DISPLAY_SCROLL_MAX_COLUMNS)
                      ? DISPLAY_SCROLL_MAX_COLUMNS
                      : len * CHAR_WIDTH;
      display_set_mode_scroll_packed(display_set_mode_internal_scroll_buffer,
                                     n_col, display_set_mode_speed);
      display_set_mode_state = SET_MODE_IDLE;
      return;
    }
    default:
      my_assert(false);
  }
}

void display_set_mode_text(const char *text) {
  display_buf_t buf[DISPLAY_WIDTH];
  int len = strlen(text);
  int i;
  for (i = 0; i < len && i * CHAR_WIDTH < DISPLAY_WIDTH; ++i) {
    display_buf_render_char(buf, text[i], i * CHAR_WIDTH, 0, DISPLAY_WIDTH,
                            DISPLAY_HEIGHT);
  }

  // clean rest garbage
  for (i = i * CHAR_WIDTH; i < DISPLAY_WIDTH; ++i) buf[i] = 0;

  display_set_mode_fixed_packed(buf);
  display_set_mode_state = SET_MODE_IDLE;
}

void display_set_mode_editor(hitcon::TextEditorDisplay *editor) {
  display_mode = DISPLAY_MODE_TEXT_EDITOR;
  text_editor_display = editor;
  display_set_mode_state = SET_MODE_IDLE;
}

void display_set_orientation(int orientation) {
  display_set_mode_orientation = orientation;
}

int display_get_scroll_count() {
  if (display_mode != DISPLAY_MODE_SCROLL) {
    return -1;
  }

  int total_width = DISPLAY_WIDTH + display_scroll_info.n_col + 1;
  int period = total_width * display_scroll_info.speed;
  int frame = display_current_frame - display_scroll_info.first_frame;
  return frame / period;
}
