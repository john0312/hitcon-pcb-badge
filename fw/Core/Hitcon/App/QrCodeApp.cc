#include "QrCodeApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/NvStorage.h>

#include <cstdint>

namespace hitcon {
namespace app {
namespace qr {

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

// QR code is split into 3 vertical thirds derived from kQrPatternWidth.
// Bit N of qr_unlocked_mask = 1 means the Nth third is unlocked.
inline bool IsUnlocked(int col, uint8_t mask) {
  int third = col * 3 / kQrPatternWidth;
  return (mask & (1 << third)) != 0;
}

// For each column, a 32-bit bitmap of rows that belong to a finder + separator
// region. These rows always render from kQrPattern regardless of lock state so
// the screen is always recognizable as a QR code.
//   cols 0..7  : TL (rows 0-7) + BL (rows 24-31)
//   cols 24..31: TR (rows 0-7)
//   otherwise  : none
inline uint32_t FinderBitsForCol(int col) {
  if (col < 8) return 0xFF0000FFu;
  if (col >= 24) return 0x000000FFu;
  return 0;
}

}  // namespace

void QrCodeApp::OnEntry() {
  offset_x_ = 0;
  offset_y_ = 0;
  unlocked_mask_ = g_nv_storage.GetCurrentStorage().qr_unlocked_mask;
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
    case BUTTON_OK:
#ifdef DEBUG
      if (mode_key_down_) {
        nv_storage_content &nv = g_nv_storage.GetCurrentStorage();
        nv.qr_unlocked_mask = 0;
        g_nv_storage.MarkDirty();
        unlocked_mask_ = 0;
        Render();
      } else {
        // cycle mask 0..7
        nv_storage_content &nv = g_nv_storage.GetCurrentStorage();
        nv.qr_unlocked_mask = (nv.qr_unlocked_mask + 1) % 8;
        g_nv_storage.MarkDirty();
        unlocked_mask_ = nv.qr_unlocked_mask;
        Render();
      }
#endif  // DEBUG
      break;
    case BUTTON_BACK:
    case BUTTON_LONG_BACK:
      badge_controller.BackToMenu(this);
      break;
    default:
      break;
  }
}

void QrCodeApp::OnEdgeButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_MODE:
      if (button & BUTTON_KEYDOWN_BIT) {
        mode_key_down_ = true;
      } else if (button & BUTTON_KEYUP_BIT) {
        mode_key_down_ = false;
      }
      break;
  }
}

void QrCodeApp::Render() {
  display_buf_t buf[DISPLAY_WIDTH] = {0};
  const int byte_idx = offset_y_ >> 3;
  const int shift = offset_y_ & 7;
  for (int x = 0; x < DISPLAY_WIDTH; ++x) {
    const int src_col = x + offset_x_;
    const uint8_t *col = kQrPattern[src_col];
    const uint8_t data_byte =
        (shift == 0) ? col[byte_idx]
                     : static_cast<uint8_t>((col[byte_idx] >> shift) |
                                            (col[byte_idx + 1] << (8 - shift)));
    if (IsUnlocked(src_col, unlocked_mask_)) {
      buf[x] = data_byte;
    } else {
      const uint8_t finder_mask = static_cast<uint8_t>(
          (FinderBitsForCol(src_col) >> offset_y_) & 0xFFu);
      // 0x11: 1 bit on and 3 bits off, to render the locked pattern
      const uint8_t phase = static_cast<uint8_t>(src_col - offset_y_) & 3u;
      const uint8_t stripe_byte = 0x11u << phase;
      buf[x] = static_cast<uint8_t>((data_byte & finder_mask) |
                                    (stripe_byte & ~finder_mask));
    }
  }
  display_set_mode_fixed_packed(buf);
}

}  // namespace qr
}  // namespace app
}  // namespace hitcon
