#ifndef HITCON_SECRET_SECRET_H
#define HITCON_SECRET_SECRET_H

#include <Logic/ButtonLogic.h>

namespace hitcon {
constexpr button_t COMBO_BUTTON[] = {
    BUTTON_UP,    BUTTON_UP,   BUTTON_DOWN,  BUTTON_DOWN,       BUTTON_LEFT,
    BUTTON_RIGHT, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_BRIGHTNESS, BUTTON_MODE,
};
constexpr button_t COMBO_BUTTON_DINO[] = {
    BUTTON_DOWN, BUTTON_DOWN, BUTTON_RIGHT, BUTTON_RIGHT, BUTTON_OK};
constexpr size_t COMBO_BUTTON_LEN = sizeof(COMBO_BUTTON) / sizeof(button_t);
constexpr size_t COMBO_BUTTON_DINO_LEN =
    sizeof(COMBO_BUTTON_DINO) / sizeof(button_t);
extern int combo_button_ctr;

#define DATA(col) (col), 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08

constexpr uint8_t kGameAchievementData[] = {
    // First byte is the column
    // 0x00,
    // Next 8 bytes is the data to give to AcceptData().
    // 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    // Repeat...

    // clang-format off
    // Tetris
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 15
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 16
    0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 17
    0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 18
    0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 20
    0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 22
    0x03, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 24
    0x03, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 26
    0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 28
    0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 30
    0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 31
    0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 32
    0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 33
    0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 34
    0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 35
    0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 37

    // Dino
    0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 15
    0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 16
    0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 17
    0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 18
    0x0a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 20
    0x0a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 22
    0x0b, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 24
    0x0b, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 26
    0x0c, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 28
    0x0c, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 30
    0x0d, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 31
    0x0d, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 32
    0x0e, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 33
    0x0e, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 34
    0x0f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 35
    0x0f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 37

    // Snake
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 10
    0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 10
    0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 12
    0x03, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 12
    0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 14
    0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 15
    0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 16
    0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 18
    0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 20
    0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 22
    0x0a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 24
    0x0b, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 26
    0x0c, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 28
    0x0d, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 30
    0x0e, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 32
    0x0f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 38
    // clang-format on
};
constexpr size_t kGameAchievementDataSize =
    sizeof(kGameAchievementData) / sizeof(kGameAchievementData[0]);
static_assert(kGameAchievementDataSize % 9 == 0);
constexpr size_t kGameAchievementDataCount = kGameAchievementDataSize / 9;

constexpr uint32_t kPerDataPeriod = 5 * 60;
constexpr uint8_t kXBoardWindowSize = 4;

constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x0a), // 15
    DATA(0x0b), // 15
    DATA(0x0c), // 15
    DATA(0x0f), // 15
    DATA(0x0a), // 15
    DATA(0x0b), // 15
    DATA(0x0c), // 15
    DATA(0x0f), // 15
    DATA(0x0a), // 16
    DATA(0x0b), // 16
    DATA(0x0c), // 16
    DATA(0x0f), // 16
    DATA(0x0a), // 16
    DATA(0x0b), // 16
    DATA(0x0c), // 16
    DATA(0x0f), // 16

    /* PARTITION 2 */
    DATA(0x0a), // 17
    DATA(0x0b), // 17
    DATA(0x0c), // 17
    DATA(0x0f), // 17
    DATA(0x0a), // 17
    DATA(0x0b), // 17
    DATA(0x0c), // 17
    DATA(0x0f), // 17
    DATA(0x0a), // 18
    DATA(0x0b), // 18
    DATA(0x0c), // 18
    DATA(0x0f), // 18
    DATA(0x0a), // 18
    DATA(0x0b), // 18
    DATA(0x0c), // 18
    DATA(0x0f), // 18

    /* PARTITION 3 */
    DATA(0x0a), // 19
    DATA(0x0b), // 19
    DATA(0x0c), // 19
    DATA(0x0f), // 19
    DATA(0x0a), // 19
    DATA(0x0b), // 19
    DATA(0x0c), // 19
    DATA(0x0f), // 19
    DATA(0x0a), // 20
    DATA(0x0b), // 20
    DATA(0x0c), // 20
    DATA(0x0f), // 20
    DATA(0x0a), // 20
    DATA(0x0b), // 20
    DATA(0x0c), // 20
    DATA(0x0f), // 20

    /* PARTITION 4 */
    DATA(0x0a), // 21
    DATA(0x0b), // 21
    DATA(0x0c), // 21
    DATA(0x0f), // 21
    DATA(0x0a), // 21
    DATA(0x0b), // 21
    DATA(0x0c), // 21
    DATA(0x0f), // 21
    DATA(0x0a), // 22
    DATA(0x0b), // 22
    DATA(0x0c), // 22
    DATA(0x0f), // 22
    DATA(0x0a), // 22
    DATA(0x0b), // 22
    DATA(0x0c), // 22
    DATA(0x0f), // 22
    // clang-format on
};

constexpr uint8_t kStrayXBoardData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x0a), // 17
    DATA(0x0b), // 17
    DATA(0x0c), // 17
    DATA(0x0d), // 17
    DATA(0x0e), // 17
    DATA(0x0f), // 17
    DATA(0x0a), // 17
    DATA(0x0b), // 17
    DATA(0x0c), // 18
    DATA(0x0d), // 18
    DATA(0x0e), // 18
    DATA(0x0f), // 18
    DATA(0x0a), // 18
    DATA(0x0b), // 18
    DATA(0x0c), // 18
    DATA(0x0d), // 18

    /* PARTITION 2 */
    DATA(0x0e), // 19
    DATA(0x0f), // 19
    DATA(0x0a), // 19
    DATA(0x0b), // 19
    DATA(0x0c), // 19
    DATA(0x0d), // 19
    DATA(0x0e), // 19
    DATA(0x0f), // 19
    DATA(0x0a), // 20
    DATA(0x0b), // 20
    DATA(0x0c), // 20
    DATA(0x0d), // 20
    DATA(0x0e), // 20
    DATA(0x0f), // 20
    DATA(0x0a), // 20
    DATA(0x0b), // 20

    /* PARTITION 3 */
    DATA(0x0c), // 21
    DATA(0x0d), // 21
    DATA(0x0e), // 21
    DATA(0x0f), // 21
    DATA(0x0a), // 21
    DATA(0x0b), // 21
    DATA(0x0c), // 21
    DATA(0x0d), // 21
    DATA(0x0e), // 22
    DATA(0x0f), // 22
    DATA(0x0a), // 22
    DATA(0x0b), // 22
    DATA(0x0c), // 22
    DATA(0x0d), // 22
    DATA(0x0e), // 22
    DATA(0x0f), // 22

    /* PARTITION 4 */
    DATA(0x0a), // 23
    DATA(0x0b), // 23
    DATA(0x0c), // 23
    DATA(0x0d), // 23
    DATA(0x0e), // 23
    DATA(0x0f), // 23
    DATA(0x0a), // 23
    DATA(0x0b), // 23
    DATA(0x0c), // 24
    DATA(0x0d), // 24
    DATA(0x0e), // 24
    DATA(0x0f), // 24
    DATA(0x0a), // 24
    DATA(0x0b), // 24
    DATA(0x0c), // 24
    DATA(0x0d), // 24
    // clang-format on
};

#ifdef DATA
#undef DATA
#endif

constexpr int kPartitionOffset = 0x0;

constexpr uint8_t kPartitionPacketId = 0x4;

constexpr size_t kPartitionCount = 4;

constexpr size_t kIrPartitionSize =
    sizeof(kStrayIRData) / sizeof(kStrayIRData[0]) / kPartitionCount / 9;
constexpr size_t kXBoardPartitionSize = sizeof(kStrayXBoardData) /
                                        sizeof(kStrayXBoardData[0]) /
                                        kPartitionCount / 9;

// From 0 to 0x10000, 0x10000 is using it 100% of the time.
constexpr int kIrUsePreparedChance = 0x8000;
// From 0 to 0x10000, 0x10000 is using it 100% of the time.
constexpr int kXBoardUsePreparedChance = 0x8000;

}  // namespace hitcon
#endif  // HITCON_SECRET_SECRET_H
