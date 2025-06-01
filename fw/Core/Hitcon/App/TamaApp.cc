#include "TamaApp.h"

#include <Logic/BadgeController.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {
namespace app {
namespace tama {
TamaApp tama_app;

TamaApp::TamaApp()
    : _routine_task(930,
                    (hitcon::service::sched::task_callback_t)&TamaApp::Routine,
                    (void*)this, INTERVAL),
      _tama_data(g_nv_storage.GetCurrentStorage().tama_storage) {}

void TamaApp::Init() {
  hitcon::service::sched::scheduler.Queue(&_routine_task, nullptr);
}

void TamaApp::OnEntry() {
  display_buf_t frame_buf[DISPLAY_WIDTH] = {0};

  // Draw the dog bitmap (columns 0-7 of frame_buf)
  // 'i' will be the column index within the 8x8 bitmap (0 to 7)
  // 'j' will be the row index within the 8x8 bitmap (0 to 7)
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      // Get the pixel value (0 or 1) from the dog bitmap.
      // tama_dog_bitmap[j] is the data for row 'j'.
      // (7 - i) accesses the i-th pixel from the left (MSB is pixel 0, LSB is pixel 7).
      uint8_t pixel_value = display_buf_get(tama_dog_bitmap[j], 7 - i);

      if (pixel_value) {
        // If the pixel is set, set the corresponding bit in the display buffer.
        // frame_buf[i] is the i-th column of the display.
        // (1 << j) creates a mask for the j-th row.
        frame_buf[i] |= (1 << j);
      }
    }
  }

  // Draw the cat bitmap (columns 8-15 of frame_buf)
  // 'i' will be the column index within the 8x8 bitmap (0 to 7)
  // 'j' will be the row index within the 8x8 bitmap (0 to 7)
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t display_column = 8 + i; // Offset for the cat's position
    if (display_column >= DISPLAY_WIDTH) {
        // Avoid writing out of bounds if display is not wide enough
        break;
    }
    for (uint8_t j = 0; j < 8; j++) {
      // Get the pixel value (0 or 1) from the cat bitmap.
      uint8_t pixel_value = display_buf_get(tama_cat_bitmap[j], 7 - i);

      if (pixel_value) {
        // Set the corresponding bit in the display buffer for the cat.
        // frame_buf[display_column] is the target column for the cat.
        // (1 << j) creates a mask for the j-th row.
        frame_buf[display_column] |= (1 << j);
      }
    }
  }
  display_set_mode_fixed_packed(frame_buf);

}

void TamaApp::OnExit() {}

void TamaApp::OnButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      break;
    default:
      break;
  }
  return;
}

void TamaApp::OnEdgeButton(button_t button) {}

void TamaApp::Routine(void* unused) {}
}  // namespace tama
}  // namespace app
}  // namespace hitcon
