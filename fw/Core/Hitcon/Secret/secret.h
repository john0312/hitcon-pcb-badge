#ifndef HITCON_SECRET_SECRET_H
#define HITCON_SECRET_SECRET_H

#include <Logic/ButtonLogic.h>

namespace hitcon {
constexpr button_t COMBO_BUTTON[] = {
    BUTTON_UP,    BUTTON_UP,   BUTTON_DOWN,  BUTTON_DOWN,       BUTTON_LEFT,
    BUTTON_RIGHT, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_BRIGHTNESS, BUTTON_MODE,
};
constexpr size_t COMBO_BUTTON_LEN = sizeof(COMBO_BUTTON) / sizeof(button_t);
extern int combo_button_ctr;

constexpr uint8_t kGameAchievementData[] = {
    // First byte is the column
    0x00,
    // Next 8 bytes is the data to give to AcceptData().
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    // Repeat...

    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
constexpr size_t kGameAchievementDataSize =
    sizeof(kGameAchievementData) / sizeof(kGameAchievementData[0]);
static_assert(kGameAchievementDataSize % 9 == 0);
constexpr size_t kGameAchievementDataCount = kGameAchievementDataSize / 9;

}  // namespace hitcon
#endif  // HITCON_SECRET_SECRET_H
