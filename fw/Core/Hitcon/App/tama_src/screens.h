#define IDLE_PET_WIDTH 8
#define IDLE_PET_HEIGHT 8
#define SELECT_WIDTH 8
#define SELECT_HEIGHT 8
#define SELECT_Y_OFFSET 0
#define SELECT_LEFT_X_OFFSET 0
#define SELECT_RIGHT_X_OFFSET 8
#define NUM_WIDTH 2
#define NUM_HEIGHT 5

// m_xxx = material xxx

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

namespace menu_icon {
constexpr uint8_t icon_zero[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_one[NUM_WIDTH * NUM_HEIGHT] = {
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_two[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    0, 1,  //
    1, 1,  //
    1, 0,  //
    1, 1,  //
};
constexpr uint8_t icon_three[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    0, 1,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_four[NUM_WIDTH * NUM_HEIGHT] = {
    1, 0,  //
    1, 0,  //
    1, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_five[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    1, 0,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_six[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    1, 0,  //
    1, 1,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_seven[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
    0, 1,  //
};
constexpr uint8_t icon_eight[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    1, 1,  //
    0, 0,  //
    1, 1,  //
    1, 1,  //
};
constexpr uint8_t icon_nine[NUM_WIDTH * NUM_HEIGHT] = {
    1, 1,  //
    1, 1,  //
    1, 1,  //
    0, 1,  //
    1, 1,  //
};
const uint8_t *num_icon[10] = {icon_zero,  icon_one,  icon_two, icon_three,
                               icon_four,  icon_five, icon_six, icon_seven,
                               icon_eight, icon_nine};
}  // namespace menu_icon
}  // namespace tama
}  // namespace app
}  // namespace hitcon
