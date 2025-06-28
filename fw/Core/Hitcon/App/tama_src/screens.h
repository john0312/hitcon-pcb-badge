#include "simu_setting.h"
/// -- -- -- --

#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
#define IDLE_PET_WIDTH 8
#define IDLE_PET_HEIGHT 8
#define SELECT_WIDTH 8
#define SELECT_HEIGHT 8
#define SELECT_Y_OFFSET 0
#define SELECT_LEFT_X_OFFSET 0
#define SELECT_RIGHT_X_OFFSET 8
#define NUM_WIDTH 2
#define NUM_HEIGHT 8
#define EGG_WIDTH 8
#define EGG_HEIGHT 8
#define FOOD_HEART_OVERVIEW_ICON_WIDTH 4
#define FOOD_HEART_OVERVIEW_ICON_HEIGHT 4

// m_xxx = material xxx

enum {
  IDLE_1,
  IDLE_2,
};

// the structure of compressed data
struct CompressedImage {
  uint8_t width;        // image width info
  uint8_t height;       // image height info
  const uint8_t* data;  // the pointer to the data
};

namespace hitcon {
namespace app {
namespace tama {
namespace components {

constexpr uint8_t m_dog_idle1[IDLE_PET_WIDTH * IDLE_PET_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 1, 0, 1, 0,  //
    1, 0, 0, 1, 1, 1, 1, 0,  //
    1, 0, 1, 1, 1, 1, 1, 1,  //
    1, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 0, 1, 0, 1, 0, 0,  //
};
constexpr uint8_t m_dog_idle2[IDLE_PET_WIDTH * IDLE_PET_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 1, 0, 1, 0,  //
    0, 0, 0, 1, 1, 1, 1, 0,  //
    1, 0, 1, 1, 1, 1, 1, 1,  //
    1, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 0, 1, 0, 1, 0, 0,  //
};

constexpr uint8_t m_cat_idle1[IDLE_PET_WIDTH * IDLE_PET_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 1, 0, 1, 0, 1, 0,  //
    0, 1, 0, 0, 1, 1, 1, 0,  //
    0, 1, 0, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 1, 1, 0,  //
    0, 0, 1, 0, 1, 0, 1, 0,  //
};

constexpr uint8_t m_cat_idle2[IDLE_PET_WIDTH * IDLE_PET_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    1, 1, 0, 0, 1, 0, 1, 0,  //
    0, 0, 1, 0, 1, 1, 1, 0,  //
    0, 1, 0, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 1, 1, 0,  //
    0, 0, 1, 0, 1, 0, 1, 0,  //
};

constexpr uint8_t m_select_print_all_character[DISPLAY_WIDTH * DISPLAY_HEIGHT] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,  //
        0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,  //
        0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1,  //
        0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0,  //
        0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,  //
        0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
};

constexpr uint8_t m_select_cursor[SELECT_WIDTH * SELECT_HEIGHT] = {
    0, 1, 1, 1, 1, 1, 1, 0,  //
    1, 0, 0, 0, 0, 0, 0, 1,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    1, 1, 1, 1, 1, 1, 1, 1,  //
};

}  // namespace components

namespace egg_icon {
constexpr uint8_t m_egg_75_percent_up[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 1, 0, 0, 0,  //
    0, 0, 1, 1, 1, 1, 0, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 1, 0, 0,  //
};
constexpr uint8_t m_egg_50_percent_up[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 1, 0, 0, 0,  //
    0, 0, 1, 1, 0, 1, 0, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 1, 1, 1, 1, 1, 0,  //
    0, 1, 0, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 0, 0, 0,  //
};
constexpr uint8_t m_egg_25_percent_up[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 1, 0, 0, 0,  //
    0, 0, 1, 1, 0, 1, 0, 0,  //
    0, 1, 0, 1, 0, 1, 1, 0,  //
    0, 1, 1, 1, 1, 0, 1, 0,  //
    0, 1, 0, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 0, 0, 0,  //
};
constexpr uint8_t m_egg_0_percent_up[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 0, 0, 0, 0,  //
    0, 0, 1, 1, 0, 1, 0, 0,  //
    0, 1, 0, 0, 0, 1, 1, 0,  //
    0, 1, 1, 0, 1, 0, 1, 0,  //
    0, 1, 0, 0, 0, 0, 1, 0,  //
    0, 0, 0, 0, 1, 1, 0, 0,  //
};
constexpr uint8_t m_egg_hatch_shinning1[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 0, 0, 0, 0,  //
    0, 1, 0, 1, 0, 1, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    1, 1, 0, 0, 0, 1, 1, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 1, 0, 1, 0, 1, 0, 0,  //
    0, 0, 0, 1, 0, 0, 0, 0,  //
};
constexpr uint8_t m_egg_hatch_shinning2[EGG_WIDTH * EGG_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 1, 0, 0, 0, 0,  //
    0, 0, 1, 0, 1, 0, 0, 0,  //
    0, 1, 0, 0, 0, 1, 0, 0,  //
    0, 0, 1, 0, 1, 0, 0, 0,  //
    0, 0, 0, 1, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
};
}  // namespace egg_icon

namespace menu_icon {
constexpr uint8_t icon_zero[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_one[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_two[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
    1, 0,  //
    1, 1,  //
};
constexpr uint8_t icon_three[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_four[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 0,  //
    1, 0,  //
    1, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_five[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    1, 0,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_six[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    1, 0,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_seven[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_eight[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    1, 1,  //
    0, 0,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_nine[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
const uint8_t* num_icon[10] = {icon_zero,  icon_one,  icon_two, icon_three,
                               icon_four,  icon_five, icon_six, icon_seven,
                               icon_eight, icon_nine};

constexpr uint8_t icon_important[NUM_WIDTH * NUM_HEIGHT] = {
    0, 0,  //
    0, 0,  //
    0, 0,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 0,  //
    0, 1,  //
};

constexpr uint8_t icon_status_overview_heart[FOOD_HEART_OVERVIEW_ICON_WIDTH *
                                             FOOD_HEART_OVERVIEW_ICON_HEIGHT] =
    {
        0, 0, 0, 0,  //
        0, 1, 0, 1,  //
        0, 1, 1, 1,  //
        0, 0, 1, 0,  //
};

constexpr uint8_t icon_status_overview_food[FOOD_HEART_OVERVIEW_ICON_WIDTH *
                                            FOOD_HEART_OVERVIEW_ICON_HEIGHT] = {
    0, 0, 0, 0,  //
    0, 0, 1, 0,  //
    0, 1, 0, 1,  //
    0, 0, 1, 0,  //
};

}  // namespace menu_icon

}  // namespace tama
}  // namespace app
}  // namespace hitcon
