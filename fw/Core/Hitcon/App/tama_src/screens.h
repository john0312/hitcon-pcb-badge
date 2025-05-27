#define IDLE_PET_WIDTH 8
#define IDLE_PET_HEIGHT 8
#define SELECT_WIDTH 8
#define SELECT_HEIGHT 8
#define SELECT_Y_OFFSET 0
#define SELECT_LEFT_X_OFFSET 0
#define SELECT_RIGHT_X_OFFSET 8

// m_xxx = material xxx

namespace hitcon {
namespace app {
namespace tama {
namespace screens {
// constexpr uint8_t m_dog_idle1[IDLE_PET_WIDTH * IDLE_PET_HEIGHT] = {
//     0, 0, 0, 0, 0, 0, 0, 0,  //
//     0, 0, 0, 0, 0, 0, 0, 0,  //
//     0, 0, 0, 0, 0, 0, 0, 0,  //
//     1, 0, 0, 0, 1, 0, 1, 0,  //
//     1, 0, 0, 1, 1, 1, 1, 0,  //
//     1, 1, 1, 1, 1, 1, 1, 1,  //
//     0, 1, 1, 1, 1, 1, 1, 0,  //
//     0, 1, 0, 1, 0, 1, 0, 0,  //
// };
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

}  // namespace screens
}  // namespace tama
}  // namespace app
}  // namespace hitcon
