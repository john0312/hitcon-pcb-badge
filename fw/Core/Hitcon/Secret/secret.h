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
    DATA(0x00), // 15
    DATA(0x00), // 16
    DATA(0x01), // 17
    DATA(0x01), // 18
    DATA(0x02), // 20
    DATA(0x02), // 22
    DATA(0x03), // 24
    DATA(0x03), // 26
    DATA(0x04), // 28
    DATA(0x04), // 30
    DATA(0x05), // 31
    DATA(0x05), // 32
    DATA(0x06), // 33
    DATA(0x06), // 34
    DATA(0x07), // 35
    DATA(0x07), // 37

    // Dino
    DATA(0x08), // 15
    DATA(0x08), // 16
    DATA(0x09), // 17
    DATA(0x09), // 18
    DATA(0x0a), // 20
    DATA(0x0a), // 22
    DATA(0x0b), // 24
    DATA(0x0b), // 26
    DATA(0x0c), // 28
    DATA(0x0c), // 30
    DATA(0x0d), // 31
    DATA(0x0d), // 32
    DATA(0x0e), // 33
    DATA(0x0e), // 34
    DATA(0x0f), // 35
    DATA(0x0f), // 37

    // Snake
    DATA(0x00), // 10
    DATA(0x01), // 10
    DATA(0x02), // 12
    DATA(0x03), // 12
    DATA(0x04), // 14
    DATA(0x05), // 15
    DATA(0x06), // 16
    DATA(0x07), // 18
    DATA(0x08), // 20
    DATA(0x09), // 22
    DATA(0x0a), // 24
    DATA(0x0b), // 26
    DATA(0x0c), // 28
    DATA(0x0d), // 30
    DATA(0x0e), // 32
    DATA(0x0f), // 38
    // clang-format on
};
constexpr size_t kGameAchievementDataSize =
    sizeof(kGameAchievementData) / sizeof(kGameAchievementData[0]);
static_assert(kGameAchievementDataSize % 9 == 0);
constexpr size_t kGameAchievementDataCount = kGameAchievementDataSize / 9;

constexpr uint8_t PARTITION_LEN = 4;
constexpr uint32_t PER_DATA_PERIOD = 5 * 60;
constexpr uint8_t XBOARD_WINDOW_SIZE = 4;
#if defined SPONSOR

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
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;
constexpr size_t PER_XBoard_PARTITION_SIZE =
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

// defined SPONSOR
#elif defined SPEAKER

constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x00), // 15
    DATA(0x02), // 15
    DATA(0x04), // 15
    DATA(0x06), // 15
    DATA(0x08), // 15
    DATA(0x0a), // 15
    DATA(0x0c), // 15
    DATA(0x0e), // 15
    DATA(0x00), // 16
    DATA(0x02), // 16
    DATA(0x04), // 16
    DATA(0x06), // 16
    DATA(0x08), // 16
    DATA(0x0a), // 16
    DATA(0x0c), // 16
    DATA(0x0e), // 16

    /* PARTITION 2 */
    DATA(0x00), // 17
    DATA(0x02), // 17
    DATA(0x04), // 17
    DATA(0x06), // 17
    DATA(0x08), // 17
    DATA(0x0a), // 17
    DATA(0x0c), // 17
    DATA(0x0e), // 17
    DATA(0x00), // 18
    DATA(0x02), // 18
    DATA(0x04), // 18
    DATA(0x06), // 18
    DATA(0x08), // 18
    DATA(0x0a), // 18
    DATA(0x0c), // 18
    DATA(0x0e), // 18

    /* PARTITION 3 */
    DATA(0x00), // 19
    DATA(0x02), // 19
    DATA(0x04), // 19
    DATA(0x06), // 19
    DATA(0x08), // 19
    DATA(0x0a), // 19
    DATA(0x0c), // 19
    DATA(0x0e), // 19
    DATA(0x00), // 20
    DATA(0x02), // 20
    DATA(0x04), // 20
    DATA(0x06), // 20
    DATA(0x08), // 20
    DATA(0x0a), // 20
    DATA(0x0c), // 20
    DATA(0x0e), // 20

    /* PARTITION 4 */
    DATA(0x00), // 21
    DATA(0x02), // 21
    DATA(0x04), // 21
    DATA(0x06), // 21
    DATA(0x08), // 21
    DATA(0x0a), // 21
    DATA(0x0c), // 21
    DATA(0x0e), // 21
    DATA(0x00), // 22
    DATA(0x02), // 22
    DATA(0x04), // 22
    DATA(0x06), // 22
    DATA(0x08), // 22
    DATA(0x0a), // 22
    DATA(0x0c), // 22
    DATA(0x0e), // 22
    // clang-format on
};

constexpr uint8_t kStrayXBoardData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x00), // 17
    DATA(0x01), // 17
    DATA(0x02), // 17
    DATA(0x03), // 17
    DATA(0x04), // 17
    DATA(0x05), // 17
    DATA(0x06), // 17
    DATA(0x07), // 17
    DATA(0x08), // 18
    DATA(0x09), // 18
    DATA(0x0a), // 18
    DATA(0x0b), // 18
    DATA(0x0c), // 18
    DATA(0x0d), // 18
    DATA(0x0e), // 18
    DATA(0x0f), // 18

    /* PARTITION 2 */
    DATA(0x00), // 19
    DATA(0x01), // 19
    DATA(0x02), // 19
    DATA(0x03), // 19
    DATA(0x04), // 19
    DATA(0x05), // 19
    DATA(0x06), // 19
    DATA(0x07), // 19
    DATA(0x08), // 20
    DATA(0x09), // 20
    DATA(0x0a), // 20
    DATA(0x0b), // 20
    DATA(0x0c), // 20
    DATA(0x0d), // 20
    DATA(0x0e), // 20
    DATA(0x0f), // 20

    /* PARTITION 3 */
    DATA(0x00), // 21
    DATA(0x01), // 21
    DATA(0x02), // 21
    DATA(0x03), // 21
    DATA(0x04), // 21
    DATA(0x05), // 21
    DATA(0x06), // 21
    DATA(0x07), // 21
    DATA(0x08), // 22
    DATA(0x09), // 22
    DATA(0x0a), // 22
    DATA(0x0b), // 22
    DATA(0x0c), // 22
    DATA(0x0d), // 22
    DATA(0x0e), // 22
    DATA(0x0f), // 22

    /* PARTITION 4 */
    DATA(0x00), // 23
    DATA(0x01), // 23
    DATA(0x02), // 23
    DATA(0x03), // 23
    DATA(0x04), // 23
    DATA(0x05), // 23
    DATA(0x06), // 23
    DATA(0x07), // 23
    DATA(0x08), // 24
    DATA(0x09), // 24
    DATA(0x0a), // 24
    DATA(0x0b), // 24
    DATA(0x0c), // 24
    DATA(0x0d), // 24
    DATA(0x0e), // 24
    DATA(0x0f), // 24
    // clang-format on
};
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;
constexpr size_t PER_XBoard_PARTITION_SIZE =
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

// defined SPEAKER
#elif defined HITCON_R1

constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x00), // 14
    DATA(0x01), // 14
    DATA(0x02), // 14
    DATA(0x00), // 14
    DATA(0x01), // 14
    DATA(0x02), // 14
    DATA(0x00), // 14
    DATA(0x01), // 14
    DATA(0x02), // 15
    DATA(0x00), // 15
    DATA(0x01), // 15
    DATA(0x02), // 15
    DATA(0x00), // 15
    DATA(0x01), // 15
    DATA(0x02), // 15
    DATA(0x00), // 15

    /* PARTITION 2 */
    DATA(0x01), // 16
    DATA(0x02), // 16
    DATA(0x00), // 16
    DATA(0x01), // 16
    DATA(0x02), // 16
    DATA(0x00), // 16
    DATA(0x01), // 16
    DATA(0x02), // 16
    DATA(0x00), // 17
    DATA(0x01), // 17
    DATA(0x02), // 17
    DATA(0x00), // 17
    DATA(0x01), // 17
    DATA(0x02), // 17
    DATA(0x00), // 17
    DATA(0x01), // 17

    /* PARTITION 3 */
    DATA(0x02), // 18
    DATA(0x00), // 18
    DATA(0x01), // 18
    DATA(0x02), // 18
    DATA(0x00), // 18
    DATA(0x01), // 18
    DATA(0x02), // 18
    DATA(0x00), // 18
    DATA(0x01), // 19
    DATA(0x02), // 19
    DATA(0x00), // 19
    DATA(0x01), // 19
    DATA(0x02), // 19
    DATA(0x00), // 19
    DATA(0x01), // 19
    DATA(0x02), // 19

    /* PARTITION 4 */
    DATA(0x00), // 20
    DATA(0x01), // 20
    DATA(0x02), // 20
    DATA(0x00), // 20
    DATA(0x01), // 20
    DATA(0x02), // 20
    DATA(0x00), // 20
    DATA(0x01), // 20
    DATA(0x02), // 21
    DATA(0x00), // 21
    DATA(0x01), // 21
    DATA(0x02), // 21
    DATA(0x00), // 21
    DATA(0x01), // 21
    DATA(0x02), // 21
    DATA(0x00), // 21
    // clang-format on
};
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;

constexpr uint8_t kStrayXBoardData[] = {};
constexpr size_t PER_XBoard_PARTITION_SIZE = 
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

// defined HITCON_R1
#elif defined HITCON_R2

constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x02), // 14
    DATA(0x03), // 14
    DATA(0x04), // 14
    DATA(0x02), // 14
    DATA(0x03), // 14
    DATA(0x04), // 14
    DATA(0x02), // 14
    DATA(0x03), // 14
    DATA(0x04), // 15
    DATA(0x02), // 15
    DATA(0x03), // 15
    DATA(0x04), // 15
    DATA(0x02), // 15
    DATA(0x03), // 15
    DATA(0x04), // 15
    DATA(0x02), // 15

    /* PARTITION 2 */
    DATA(0x03), // 16
    DATA(0x04), // 16
    DATA(0x02), // 16
    DATA(0x03), // 16
    DATA(0x04), // 16
    DATA(0x02), // 16
    DATA(0x03), // 16
    DATA(0x04), // 16
    DATA(0x02), // 17
    DATA(0x03), // 17
    DATA(0x04), // 17
    DATA(0x02), // 17
    DATA(0x03), // 17
    DATA(0x04), // 17
    DATA(0x02), // 17
    DATA(0x03), // 17

    /* PARTITION 3 */
    DATA(0x04), // 18
    DATA(0x02), // 18
    DATA(0x03), // 18
    DATA(0x04), // 18
    DATA(0x02), // 18
    DATA(0x03), // 18
    DATA(0x04), // 18
    DATA(0x02), // 18
    DATA(0x03), // 19
    DATA(0x04), // 19
    DATA(0x02), // 19
    DATA(0x03), // 19
    DATA(0x04), // 19
    DATA(0x02), // 19
    DATA(0x03), // 19
    DATA(0x04), // 19

    /* PARTITION 4 */
    DATA(0x02), // 20
    DATA(0x03), // 20
    DATA(0x04), // 20
    DATA(0x02), // 20
    DATA(0x03), // 20
    DATA(0x04), // 20
    DATA(0x02), // 20
    DATA(0x03), // 20
    DATA(0x04), // 21
    DATA(0x02), // 21
    DATA(0x03), // 21
    DATA(0x04), // 21
    DATA(0x02), // 21
    DATA(0x03), // 21
    DATA(0x04), // 21
    DATA(0x02), // 21
    // clang-format on
};
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;

constexpr uint8_t kStrayXBoardData[] = {};
constexpr size_t PER_XBoard_PARTITION_SIZE = 
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

// defined HITCON_R2
#elif defined HITCON_R3


constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x04), // 14
    DATA(0x05), // 14
    DATA(0x06), // 14
    DATA(0x04), // 14
    DATA(0x05), // 14
    DATA(0x06), // 14
    DATA(0x04), // 14
    DATA(0x05), // 14
    DATA(0x06), // 15
    DATA(0x04), // 15
    DATA(0x05), // 15
    DATA(0x06), // 15
    DATA(0x04), // 15
    DATA(0x05), // 15
    DATA(0x06), // 15
    DATA(0x04), // 15

    /* PARTITION 2 */
    DATA(0x05), // 16
    DATA(0x06), // 16
    DATA(0x04), // 16
    DATA(0x05), // 16
    DATA(0x06), // 16
    DATA(0x04), // 16
    DATA(0x05), // 16
    DATA(0x06), // 16
    DATA(0x04), // 17
    DATA(0x05), // 17
    DATA(0x06), // 17
    DATA(0x04), // 17
    DATA(0x05), // 17
    DATA(0x06), // 17
    DATA(0x04), // 17
    DATA(0x05), // 17

    /* PARTITION 3 */
    DATA(0x06), // 18
    DATA(0x04), // 18
    DATA(0x05), // 18
    DATA(0x06), // 18
    DATA(0x04), // 18
    DATA(0x05), // 18
    DATA(0x06), // 18
    DATA(0x04), // 18
    DATA(0x05), // 19
    DATA(0x06), // 19
    DATA(0x04), // 19
    DATA(0x05), // 19
    DATA(0x06), // 19
    DATA(0x04), // 19
    DATA(0x05), // 19
    DATA(0x06), // 19

    /* PARTITION 4 */
    DATA(0x04), // 20
    DATA(0x05), // 20
    DATA(0x06), // 20
    DATA(0x04), // 20
    DATA(0x05), // 20
    DATA(0x06), // 20
    DATA(0x04), // 20
    DATA(0x05), // 20
    DATA(0x06), // 21
    DATA(0x04), // 21
    DATA(0x05), // 21
    DATA(0x06), // 21
    DATA(0x04), // 21
    DATA(0x05), // 21
    DATA(0x06), // 21
    DATA(0x04), // 21
    // clang-format on
};
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;

constexpr uint8_t kStrayXBoardData[] = {};
constexpr size_t PER_XBoard_PARTITION_SIZE = 
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

// defined HITCONR3
#elif defined HITCON_R4

constexpr uint8_t kStrayIRData[] = {
    // First byte is the column
    // 0x00,
    // Repeat...

    // clang-format off
    /* PARTITION 1 */
    DATA(0x06), // 14
    DATA(0x07), // 14
    DATA(0x08), // 14
    DATA(0x06), // 14
    DATA(0x07), // 14
    DATA(0x08), // 14
    DATA(0x06), // 14
    DATA(0x07), // 14
    DATA(0x08), // 15
    DATA(0x06), // 15
    DATA(0x07), // 15
    DATA(0x08), // 15
    DATA(0x06), // 15
    DATA(0x07), // 15
    DATA(0x08), // 15
    DATA(0x06), // 15

    /* PARTITION 2 */
    DATA(0x07), // 16
    DATA(0x08), // 16
    DATA(0x06), // 16
    DATA(0x07), // 16
    DATA(0x08), // 16
    DATA(0x06), // 16
    DATA(0x07), // 16
    DATA(0x08), // 16
    DATA(0x06), // 17
    DATA(0x07), // 17
    DATA(0x08), // 17
    DATA(0x06), // 17
    DATA(0x07), // 17
    DATA(0x08), // 17
    DATA(0x06), // 17
    DATA(0x07), // 17

    /* PARTITION 3 */
    DATA(0x08), // 18
    DATA(0x06), // 18
    DATA(0x07), // 18
    DATA(0x08), // 18
    DATA(0x06), // 18
    DATA(0x07), // 18
    DATA(0x08), // 18
    DATA(0x06), // 18
    DATA(0x07), // 19
    DATA(0x08), // 19
    DATA(0x06), // 19
    DATA(0x07), // 19
    DATA(0x08), // 19
    DATA(0x06), // 19
    DATA(0x07), // 19
    DATA(0x08), // 19

    /* PARTITION 4 */
    DATA(0x06), // 20
    DATA(0x07), // 20
    DATA(0x08), // 20
    DATA(0x06), // 20
    DATA(0x07), // 20
    DATA(0x08), // 20
    DATA(0x06), // 20
    DATA(0x07), // 20
    DATA(0x08), // 21
    DATA(0x06), // 21
    DATA(0x07), // 21
    DATA(0x08), // 21
    DATA(0x06), // 21
    DATA(0x07), // 21
    DATA(0x08), // 21
    DATA(0x06), // 21
    // clang-format on
};
constexpr size_t PER_IR_PARTITION_SIZE =
    sizeof(kStrayIRData) / sizeof(uint8_t) / PARTITION_LEN / 9;

constexpr uint8_t kStrayXBoardData[] = {};
constexpr size_t PER_XBoard_PARTITION_SIZE = 
    sizeof(kStrayXBoardData) / sizeof(uint8_t) / PARTITION_LEN / 9;

#endif // HITCON_R4

#ifdef DATA
#undef DATA
#endif

}  // namespace hitcon
#endif  // HITCON_SECRET_SECRET_H
