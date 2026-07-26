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

constexpr int kQrPatternWidth = 21;
constexpr int kQrPatternHeight = 21;
constexpr int kQrPatternBytesPerCol = (kQrPatternHeight + 7) / 8;

// Column-major, same packing as display_buf_t: each column stored as 3 bytes,
// byte 0 holds rows 0-7 with bit 0 = row 0 (top). The unused high bits of the
// last byte (rows 21-23) stay 0.
// Placeholder content: three QR-style finder patterns at TL/TR/BL corners with
// an X across the middle data area. Real QR encoding can replace this later.
constexpr uint8_t kQrPattern[kQrPatternWidth][kQrPatternBytesPerCol] = {
    {0x7F, 0b11001001, 0x1F},              // col 0
    {0x41, 0b01010100, 0x10},              // col 1
    {0x5D, 0b01000111, 0x17},              // col 2
    {0x5D, 0b01000011, 0x17},              // col 3
    {0x5D, 0b01010001, 0x17},              // col 4
    {0x41, 0b01001011, 0x10},              // col 5
    {0x7F, 0b11010101, 0x1F},              // col 6
    {0x00, 0b00001100, 0x00},              // col 7
    {0b11111100, 0b10110100, 0b00010111},  // col 8
    {0b10000110, 0b10010101, 0b00010111},  // col 9
    {0b01000001, 0b00110101, 0b00001111},  // col 10
    {0b00100001, 0b01001001, 0b00011110},  // col 11
    {0b01011100, 0b01000110, 0b00000101},  // col 12
    {0x00, 0b10000110, 0b00001001},        // col 13
    {0x7F, 0b00010001, 0b00010001},        // col 14
    {0x41, 0b10110001, 0b00011011},        // col 15
    {0x5D, 0b10100011, 0b00000010},        // col 16
    {0x5D, 0b10001111, 0b00000010},        // col 17
    {0x5D, 0b11001001, 0b00001000},        // col 18
    {0x41, 0b11000100, 0b00010001},        // col 19
    {0x7F, 0b00111010, 0b00000001},        // col 20
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
//   cols 0..7  : TL (rows 0-7) + BL (rows 13-20)
//   cols 13..20: TR (rows 0-7)
//   otherwise  : none
inline uint32_t FinderBitsForCol(int col) {
  if (col < 8) return 0x001FE0FFu;
  if (col >= 13) return 0x000000FFu;
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
