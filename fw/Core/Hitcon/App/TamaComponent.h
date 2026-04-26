#ifndef TAMA_COMPONENT_H
#define TAMA_COMPONENT_H
#include <Logic/Display/display.h>
namespace hitcon {
namespace app {
namespace tama {

typedef struct {
  const display_buf_t* data;
  uint8_t length;
} tama_display_component_t;

// --- Display Component ---
// The data here is used to stack upon existing frames
// clang-format off
constexpr display_buf_t TAMA_PET_SELECTION_CURSOR[8] = {
  0x82, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x82};
constexpr display_buf_t TAMA_N_FONT[3] = { 0b00111100, 0b00000100, 0b00111000 };
constexpr display_buf_t TAMA_Y_FONT[3] = { 0b01011100, 0b01010000, 0b00111100 };
constexpr display_buf_t TAMA_HOSPITAL_ICONS[8] = {
  0x00, 0x18, 0x18, 0x7E, 0x7E, 0x18, 0x18, 0x00};
constexpr display_buf_t TAMA_SELECTION_CURSOR[3] = {0x80, 0x80, 0x80};
constexpr display_buf_t TAMA_NUM_ONE[3] = {0, 0, 0b11111000};
constexpr display_buf_t TAMA_NUM_TWO[3] = {0b11101000, 0b10101000, 0b10111000};
constexpr display_buf_t TAMA_NUM_THREE[3] = {0b10101000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_FOUR[3] = {0b00111000, 0b00100000, 0b11111000};
constexpr display_buf_t TAMA_NUM_FIVE[3] = {0b10111000, 0b10101000, 0b11101000};
constexpr display_buf_t TAMA_NUM_SIX[3] = {0b11111000, 0b10101000, 0b11101000};
constexpr display_buf_t TAMA_NUM_SEVEN[3] = {0b00001000, 0b00001000, 0b11111000};
constexpr display_buf_t TAMA_NUM_EIGHT[3] = {0b11111000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_NINE[3] = {0b10111000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_ZERO[3] = {0b11111000, 0b10001000, 0b11111000};
constexpr display_buf_t TAMA_QTE_WINNING_EFFECT[15] = {0x02, 0x04, 0x08, 0x01, 0x02, 0x04, 0x00, 0x03, 0x00, 0x04, 0x02, 0x01, 0x08, 0x04, 0x02};
constexpr display_buf_t TAMA_QTE_LOSING_EFFECT[11] = {0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04};
constexpr display_buf_t TAMA_TRAINING_FACILITY[6] = {0x26, 0x39, 0x26, 0x4C, 0x72, 0x4C};
constexpr display_buf_t TAMA_TRAINING_LV_UP_ONE[12] = {0x7, 0x4, 0, 0x3, 0x4, 0x3, 0, 0x2, 0x7, 0x2, 0, 0x7};
constexpr display_buf_t TAMA_TRAINING_LV_UP_TEN[16] = {0x7, 0x4, 0, 0x3, 0x4, 0x3, 0, 0x2, 0x7, 0x2, 0, 0x7, 0, 0x7, 0x5, 0x7};
// clang-format on

constexpr tama_display_component_t TAMA_COMPONENT_PET_SELECTION_CURSOR = {
    .data = TAMA_PET_SELECTION_CURSOR,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_N_FONT = {
    .data = TAMA_N_FONT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_Y_FONT = {
    .data = TAMA_Y_FONT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_SELECTION_CURSOR = {
    .data = TAMA_SELECTION_CURSOR,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_HOSPITAL_ICONS = {
    .data = TAMA_HOSPITAL_ICONS,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_ONE = {
    .data = TAMA_NUM_ONE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_TWO = {
    .data = TAMA_NUM_TWO,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_THREE = {
    .data = TAMA_NUM_THREE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_FOUR = {
    .data = TAMA_NUM_FOUR,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_FIVE = {
    .data = TAMA_NUM_FIVE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_SIX = {
    .data = TAMA_NUM_SIX,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_SEVEN = {
    .data = TAMA_NUM_SEVEN,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_EIGHT = {
    .data = TAMA_NUM_EIGHT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_NINE = {
    .data = TAMA_NUM_NINE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_ZERO = {
    .data = TAMA_NUM_ZERO,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_QTE_WINNING_EFFECT = {
    .data = TAMA_QTE_WINNING_EFFECT,
    .length = 15,
};
constexpr tama_display_component_t TAMA_COMPONENT_QTE_LOSING_EFFECT = {
    .data = TAMA_QTE_LOSING_EFFECT,
    .length = 11,
};
constexpr tama_display_component_t TAMA_COMPONENT_TRAINING_FACILITY = {
    .data = TAMA_TRAINING_FACILITY,
    .length = 6,
};
constexpr tama_display_component_t TAMA_COMPONENT_TRAINING_LV_UP_ONE = {
    .data = TAMA_TRAINING_LV_UP_ONE,
    .length = 12,
};
constexpr tama_display_component_t TAMA_COMPONENT_TRAINING_LV_UP_TEN = {
    .data = TAMA_TRAINING_LV_UP_TEN,
    .length = 16,
};

#define ASSERT_COMPONENT_PROPERTIES(TYPE_NAME_STR)                             \
  static_assert(TAMA_COMPONENT_##TYPE_NAME_STR.length ==                       \
                    sizeof(TAMA_##TYPE_NAME_STR) / sizeof(display_buf_t),      \
                #TYPE_NAME_STR " Frame count mismatch");                       \
  static_assert((TAMA_COMPONENT_##TYPE_NAME_STR).data == TAMA_##TYPE_NAME_STR, \
                #TYPE_NAME_STR " Data mismatch");

ASSERT_COMPONENT_PROPERTIES(PET_SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(N_FONT);
ASSERT_COMPONENT_PROPERTIES(Y_FONT);
ASSERT_COMPONENT_PROPERTIES(SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(HOSPITAL_ICONS);
ASSERT_COMPONENT_PROPERTIES(NUM_ONE);
ASSERT_COMPONENT_PROPERTIES(NUM_TWO);
ASSERT_COMPONENT_PROPERTIES(NUM_THREE);
ASSERT_COMPONENT_PROPERTIES(NUM_FOUR);
ASSERT_COMPONENT_PROPERTIES(NUM_FIVE);
ASSERT_COMPONENT_PROPERTIES(NUM_SIX);
ASSERT_COMPONENT_PROPERTIES(NUM_SEVEN);
ASSERT_COMPONENT_PROPERTIES(NUM_EIGHT);
ASSERT_COMPONENT_PROPERTIES(NUM_NINE);
ASSERT_COMPONENT_PROPERTIES(NUM_ZERO);
ASSERT_COMPONENT_PROPERTIES(QTE_WINNING_EFFECT);
ASSERT_COMPONENT_PROPERTIES(QTE_LOSING_EFFECT)
ASSERT_COMPONENT_PROPERTIES(TRAINING_FACILITY);
ASSERT_COMPONENT_PROPERTIES(TRAINING_LV_UP_ONE);
ASSERT_COMPONENT_PROPERTIES(TRAINING_LV_UP_TEN);

constexpr tama_display_component_t TAMA_NUM_FONT[10] = {
    TAMA_COMPONENT_NUM_ZERO,  TAMA_COMPONENT_NUM_ONE,
    TAMA_COMPONENT_NUM_TWO,   TAMA_COMPONENT_NUM_THREE,
    TAMA_COMPONENT_NUM_FOUR,  TAMA_COMPONENT_NUM_FIVE,
    TAMA_COMPONENT_NUM_SIX,   TAMA_COMPONENT_NUM_SEVEN,
    TAMA_COMPONENT_NUM_EIGHT, TAMA_COMPONENT_NUM_NINE,
};

}  // namespace tama
}  // namespace app
}  // namespace hitcon
#endif  // TAMA_COMPONENT_H
