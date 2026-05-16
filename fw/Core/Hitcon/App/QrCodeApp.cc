#include "QrCodeApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>

#include <cstdint>

namespace hitcon {

QrCodeApp qr_code_app;

namespace {

constexpr int kQrPatternWidth = 32;
constexpr int kQrPatternHeight = 32;
constexpr int kQrPatternBytesPerCol = kQrPatternHeight / 8;

// Column-major, same packing as display_buf_t: each column stored as 4 bytes,
// byte 0 holds rows 0-7 with bit 0 = row 0 (top).
// Placeholder content: three QR-style finder patterns at TL/TR/BL corners with
// an X across the middle data area. Real QR encoding can replace this later.
constexpr uint8_t kQrPattern[kQrPatternWidth][kQrPatternBytesPerCol] = {
    {0x7F, 0x00, 0x00, 0xFE},  // col 0
    {0x41, 0x01, 0x00, 0x82},  // col 1
    {0x5D, 0x02, 0x00, 0xBA},  // col 2
    {0x5D, 0x04, 0x00, 0xBA},  // col 3
    {0x5D, 0x08, 0x00, 0xBA},  // col 4
    {0x41, 0x10, 0x00, 0x82},  // col 5
    {0x7F, 0x20, 0x00, 0xFE},  // col 6
    {0x00, 0x40, 0x00, 0x00},  // col 7
    {0x00, 0x80, 0x00, 0x00},  // col 8
    {0x00, 0x00, 0x01, 0x00},  // col 9
    {0x00, 0x00, 0x02, 0x00},  // col 10
    {0x00, 0x00, 0x04, 0x00},  // col 11
    {0x00, 0x00, 0x08, 0x00},  // col 12
    {0x00, 0x00, 0x10, 0x00},  // col 13
    {0x00, 0x00, 0x20, 0x01},  // col 14
    {0x00, 0x00, 0xC0, 0x00},  // col 15  X meets center
    {0x00, 0x00, 0xC0, 0x00},  // col 16
    {0x00, 0x00, 0x20, 0x01},  // col 17
    {0x00, 0x00, 0x10, 0x00},  // col 18
    {0x00, 0x00, 0x08, 0x00},  // col 19
    {0x00, 0x00, 0x04, 0x00},  // col 20
    {0x00, 0x00, 0x02, 0x00},  // col 21
    {0x00, 0x00, 0x01, 0x00},  // col 22
    {0x00, 0x80, 0x00, 0x00},  // col 23
    {0x00, 0x40, 0x00, 0x00},  // col 24
    {0x7F, 0x20, 0x00, 0x00},  // col 25
    {0x41, 0x10, 0x00, 0x00},  // col 26
    {0x5D, 0x08, 0x00, 0x00},  // col 27
    {0x5D, 0x04, 0x00, 0x00},  // col 28
    {0x5D, 0x02, 0x00, 0x00},  // col 29
    {0x41, 0x01, 0x00, 0x00},  // col 30
    {0x7F, 0x00, 0x00, 0x00},  // col 31
};

constexpr int kMaxOffsetX = kQrPatternWidth - DISPLAY_WIDTH;
constexpr int kMaxOffsetY = kQrPatternHeight - DISPLAY_HEIGHT;

}  // namespace

void QrCodeApp::OnEntry() {
  offset_x_ = 0;
  offset_y_ = 0;
  Render();
}

void QrCodeApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_LEFT:
      if (offset_x_ > 0) {
        --offset_x_;
        Render();
      }
      break;
    case BUTTON_RIGHT:
      if (offset_x_ < kMaxOffsetX) {
        ++offset_x_;
        Render();
      }
      break;
    case BUTTON_UP:
      if (offset_y_ > 0) {
        --offset_y_;
        Render();
      }
      break;
    case BUTTON_DOWN:
      if (offset_y_ < kMaxOffsetY) {
        ++offset_y_;
        Render();
      }
      break;
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      break;
    default:
      break;
  }
}

void QrCodeApp::Render() {
  display_buf_t buf[DISPLAY_WIDTH] = {0};
  const int byte_idx = offset_y_ >> 3;
  const int shift = offset_y_ & 7;
  for (int x = 0; x < DISPLAY_WIDTH; ++x) {
    const uint8_t *col = kQrPattern[x + offset_x_];
    if (shift == 0) {
      buf[x] = col[byte_idx];
    } else {
      buf[x] = static_cast<uint8_t>((col[byte_idx] >> shift) |
                                    (col[byte_idx + 1] << (8 - shift)));
    }
  }
  display_set_mode_fixed_packed(buf);
}

}  // namespace hitcon
