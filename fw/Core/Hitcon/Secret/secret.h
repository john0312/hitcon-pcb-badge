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
    DATA(0x0c), // 15
    DATA(0x03), // 16
    DATA(0x01), // 17
    DATA(0x08), // 18
    DATA(0x00), // 20
    DATA(0x09), // 22
    DATA(0x07), // 24
    DATA(0x04), // 26
    DATA(0x05), // 28
    DATA(0x0d), // 30
    DATA(0x07), // 31
    DATA(0x09), // 32
    DATA(0x0d), // 33
    DATA(0x08), // 34
    DATA(0x0e), // 35
    DATA(0x03), // 37

    // Dino
    DATA(0x05), // 15
    DATA(0x0a), // 16
    DATA(0x0c), // 17
    DATA(0x01), // 18
    DATA(0x05), // 20
    DATA(0x08), // 22
    DATA(0x09), // 24
    DATA(0x00), // 26
    DATA(0x0a), // 28
    DATA(0x0f), // 30
    DATA(0x0d), // 31
    DATA(0x02), // 32
    DATA(0x02), // 33
    DATA(0x06), // 34
    DATA(0x01), // 35
    DATA(0x0f), // 37

    // Snake
    DATA(0x05), // 10
    DATA(0x0a), // 10
    DATA(0x0c), // 12
    DATA(0x01), // 12
    DATA(0x05), // 14
    DATA(0x08), // 15
    DATA(0x09), // 16
    DATA(0x00), // 18
    DATA(0x0a), // 20
    DATA(0x0f), // 22
    DATA(0x0d), // 24
    DATA(0x02), // 26
    DATA(0x02), // 28
    DATA(0x06), // 30
    DATA(0x01), // 32
    DATA(0x0f), // 38

    // clang-format on
};
constexpr size_t kGameAchievementDataSize =
    sizeof(kGameAchievementData) / sizeof(kGameAchievementData[0]);
static_assert(kGameAchievementDataSize % 9 == 0);
constexpr size_t kGameAchievementDataCount = kGameAchievementDataSize / 9;

constexpr uint32_t kPerDataPeriod = 5 * 60;
constexpr uint8_t kXBoardWindowSize = 4;

#define ATTENDEE

#if defined SPONSOR

constexpr bool kForceShowNameOnly = true;
constexpr bool kLegacySurpriseBehaviour = true;

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

#elif defined SPEAKER

constexpr bool kForceShowNameOnly = false;
constexpr bool kLegacySurpriseBehaviour = true;

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

#elif defined HITCON_R1

constexpr bool kForceShowNameOnly = true;
constexpr bool kLegacySurpriseBehaviour = false;

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
};

#elif defined HITCON_R2

constexpr bool kForceShowNameOnly = true;
constexpr bool kLegacySurpriseBehaviour = false;

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
};

#elif defined HITCON_R3

constexpr bool kForceShowNameOnly = true;
constexpr bool kLegacySurpriseBehaviour = false;

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

constexpr uint8_t kStrayXBoardData[] = {};

#elif defined HITCON_R4

constexpr bool kForceShowNameOnly = true;
constexpr bool kLegacySurpriseBehaviour = false;

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

constexpr uint8_t kStrayXBoardData[] = {};

#elif defined ATTENDEE

constexpr uint8_t kStrayIRData[] = {};

constexpr uint8_t kStrayXBoardData[] = {};

constexpr bool kForceShowNameOnly = false;
constexpr bool kLegacySurpriseBehaviour = false;

#else

static_assert(false);

#endif // HITCON_R4

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
