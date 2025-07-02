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
#define WEAK_PET_WIDTH 8
#define WEAK_PET_HEIGHT 8
#define WEAK_PET_PARTICLE_WIDTH 8
#define WEAK_PET_PARTICLE_HEIGHT 8
#define HOSPITAL_WIDTH 8
#define HOSPITAL_HEIGHT 8
#define BATTLE_WIDTH 7
#define BATTLE_HEIGHT 8
#define TRAINING_WIDTH 7
#define TRAINING_HEIGHT 8
#define YN_WIDTH 16
#define YN_HEIGHT 8
#define YN_SELECT_LEFT_WIDTH 4
#define YN_SELECT_LEFT_HEIGHT 8
#define YN_SELECT_RIGHT_WIDTH 3
#define YN_SELECT_RIGHT_HEIGHT 8

// m_xxx = material xxx

enum {
  FRAME_1,
  FRAME_2,
};

enum {
  PET_TYPE_DOG,
  PET_TYPE_CAT,
};

enum {
  BATTLE,
  TRAINING,
};

enum {
  LEFT,
  RIGHT,
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

/**
 * @brief The compressed data of dog idle1.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 1, 0, 1, 0,  //
 *  1, 0, 0, 1, 1, 1, 1, 0,  //
 *  1, 0, 1, 1, 1, 1, 1, 1,  //
 *  1, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 0, 1, 0, 1, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_dog_idle1_compressed_data[] = {0x38, 0xE0, 0x70, 0xF8,
                                                   0x7C, 0xF8, 0x7C, 0x10};
constexpr CompressedImage m_dog_idle1_compressed = {
    .width = IDLE_PET_WIDTH,
    .height = IDLE_PET_HEIGHT,
    .data = m_dog_idle1_compressed_data};

/**
 * @brief The compressed data of dog idle2.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 1, 0, 1, 0,  //
 *  0, 0, 0, 1, 1, 1, 1, 0,  //
 *  1, 0, 1, 1, 1, 1, 1, 1,  //
 *  1, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 0, 1, 0, 1, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_dog_idle2_compressed_data[] = {0x30, 0xE0, 0x70, 0xF8,
                                                   0x7C, 0xF8, 0x7C, 0x10};
constexpr CompressedImage m_dog_idle2_compressed = {
    .width = IDLE_PET_WIDTH,
    .height = IDLE_PET_HEIGHT,
    .data = m_dog_idle2_compressed_data};

/**
 * @brief The compressed data of m_cat_idle1.
 *
 * The original data is:
 *
 *  ```
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 0, 0, 0, 0, 0, 0,  //
    0, 0, 1, 0, 1, 0, 1, 0,  //
    0, 1, 0, 0, 1, 1, 1, 0,  //
    0, 1, 0, 1, 1, 1, 1, 0,  //
    0, 0, 1, 1, 1, 1, 1, 0,  //
    0, 0, 1, 0, 1, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_cat_idle1_compressed_data[] = {0x00, 0x30, 0xC8, 0x60,
                                                   0xF8, 0x70, 0xF8, 0x00};
constexpr CompressedImage m_cat_idle1_compressed = {
    .width = IDLE_PET_WIDTH,
    .height = IDLE_PET_HEIGHT,
    .data = m_cat_idle1_compressed_data};

/**
 * @brief The compressed data of m_cat_idle2.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 0, 0, 1, 0, 1, 0,  //
 *  0, 0, 1, 0, 1, 1, 1, 0,  //
 *  0, 1, 0, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 0, 1, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_cat_idle2_compressed_data[] = {0x08, 0x28, 0xD0, 0x60,
                                                   0xF8, 0x70, 0xF8, 0x00};
constexpr CompressedImage m_cat_idle2_compressed = {
    .width = IDLE_PET_WIDTH,
    .height = IDLE_PET_HEIGHT,
    .data = m_cat_idle2_compressed_data};

/**
 * @brief The compressed data of m_select_print_all_character.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,  //
 *  0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,  //
 *  0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1,  //
 *  0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_select_print_all_character_compressed_data[] = {
    0x00, 0x18, 0x60, 0x30, 0x7C, 0x38, 0x7C, 0x00,
    0x18, 0x70, 0x38, 0x7E, 0x3C, 0x7E, 0x3C, 0x08};
constexpr CompressedImage m_select_print_all_character_compressed = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .data = m_select_print_all_character_compressed_data};

/**
 * @brief The compressed data of m_select_cursor.
 *
 * The original data is:
 *
 *  ```
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  1, 0, 0, 0, 0, 0, 0, 1,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 1, 1, 1, 1, 1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_select_cursor_compressed_data[] = {0x82, 0x81, 0x81, 0x81,
                                                       0x81, 0x81, 0x81, 0x82};
constexpr CompressedImage m_select_cursor_compressed = {
    .width = SELECT_WIDTH,
    .height = SELECT_HEIGHT,
    .data = m_select_cursor_compressed_data};

/**
 * @brief The compressed data of m_dog_weak_compressed_data.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 0, 1, 0, 0,  //
 *  1, 1, 0, 1, 1, 1, 0, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 */
constexpr uint8_t m_dog_weak_compressed_data[] = {0x20, 0xE0, 0xC0, 0xF0,
                                                  0xE0, 0xF0, 0x40, 0x00};
constexpr CompressedImage m_dog_weak_compressed = {
    .width = WEAK_PET_WIDTH,
    .height = WEAK_PET_HEIGHT,
    .data = m_dog_weak_compressed_data};

/**
 * @brief The compressed data of m_cat_weak_compressed_data.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 0, 0, 1, 0, 1, 0,  //
 *  0, 0, 1, 0, 1, 1, 1, 0,  //
 *  0, 1, 0, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 1, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_cat_weak_compressed_data[] = {0x10, 0x50, 0xA0, 0xC0,
                                                  0xF0, 0xE0, 0xF0, 0x00};
constexpr CompressedImage m_cat_weak_compressed = {
    .width = WEAK_PET_WIDTH,
    .height = WEAK_PET_HEIGHT,
    .data = m_cat_weak_compressed_data};

/**
 * @brief The compressed data of m_weak_particle_1_compressed_data.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 1, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 1, 0,  //
 *  0, 0, 1, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_weak_particle_1_compressed_data[] = {
    0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x04, 0x00};
constexpr CompressedImage m_weak_particle_1_compressed = {
    .width = WEAK_PET_PARTICLE_WIDTH,
    .height = WEAK_PET_PARTICLE_HEIGHT,
    .data = m_weak_particle_1_compressed_data};

/**
 * @brief The compressed data of m_weak_particle_2_compressed_data.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 1, 0,  //
 *  0, 0, 1, 0, 1, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_weak_particle_2_compressed_data[] = {
    0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x02, 0x00};
constexpr CompressedImage m_weak_particle_2_compressed = {
    .width = WEAK_PET_PARTICLE_WIDTH,
    .height = WEAK_PET_PARTICLE_HEIGHT,
    .data = m_weak_particle_2_compressed_data};

}  // namespace components

namespace egg_icon {

/**
 * @brief The compressed data of m_egg_75_percent_up.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 1, 1, 1, 1, 0, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 1, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_75_percent_up_compressed_data[] = {
    0x00, 0x70, 0xF8, 0xFC, 0xFC, 0xF8, 0x70, 0x00};
constexpr CompressedImage m_egg_75_percent_up_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_75_percent_up_compressed_data};

/**
 * @brief The compressed data of m_egg_50_percent_up.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 1, 1, 0, 1, 0, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 0, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_50_percent_up_compressed_data[] = {
    0x00, 0x70, 0xB8, 0xFC, 0xF4, 0x78, 0x70, 0x00};
constexpr CompressedImage m_egg_50_percent_up_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_50_percent_up_compressed_data};

/**
 * @brief The compressed data of m_egg_25_percent_up.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 1, 1, 0, 1, 0, 0,  //
 *  0, 1, 0, 1, 0, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 0, 1, 0,  //
 *  0, 1, 0, 1, 1, 1, 1, 0,  //
 *  0, 0, 1, 1, 1, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_25_percent_up_compressed_data[] = {
    0x00, 0x70, 0xA8, 0xFC, 0xE4, 0x58, 0x70, 0x00};
constexpr CompressedImage m_egg_25_percent_up_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_25_percent_up_compressed_data};

/**
 * @brief The compressed data of m_egg_0_percent_up.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 0, 1, 1, 0, 1, 0, 0,  //
 *  0, 1, 0, 0, 0, 1, 1, 0,  //
 *  0, 1, 1, 0, 1, 0, 1, 0,  //
 *  0, 1, 0, 0, 0, 0, 1, 0,  //
 *  0, 0, 0, 0, 1, 1, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_0_percent_up_compressed_data[] = {
    0x00, 0x70, 0x28, 0x0C, 0xA0, 0x98, 0x70, 0x00};
constexpr CompressedImage m_egg_0_percent_up_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_0_percent_up_compressed_data};

/**
 * @brief The compressed data of m_egg_hatch_shinning1.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 1, 0, 1, 0, 1, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 0, 0, 0, 1, 1, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 1, 0, 1, 0, 1, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_hatch_shinning1_compressed_data[] = {
    0x10, 0x54, 0x00, 0xC6, 0x00, 0x54, 0x10, 0x00};
constexpr CompressedImage m_egg_hatch_shinning1_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_hatch_shinning1_compressed_data};

/**
 * @brief The compressed data of m_egg_hatch_shinning2.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 0, 1, 0, 1, 0, 0, 0,  //
 *  0, 1, 0, 0, 0, 1, 0, 0,  //
 *  0, 0, 1, 0, 1, 0, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_egg_hatch_shinning2_compressed_data[] = {
    0x00, 0x10, 0x28, 0x44, 0x28, 0x10, 0x00, 0x00};
constexpr CompressedImage m_egg_hatch_shinning2_compressed = {
    .width = EGG_WIDTH,
    .height = EGG_HEIGHT,
    .data = m_egg_hatch_shinning2_compressed_data};
}  // namespace egg_icon

namespace menu_icon {

/**
 * @brief The compressed data of m_icon_zero.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 1,  //
 *  1, 1,  //
 *  1, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_zero_compressed_data[] = {0xF8, 0xF8};
constexpr CompressedImage m_icon_zero_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_zero_compressed_data};

/**
 * @brief The compressed data of m_icon_one.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_one_compressed_data[] = {0x00, 0xF8};
constexpr CompressedImage m_icon_one_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_one_compressed_data};

/**
 * @brief The compressed data of m_icon_two.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  0, 1,  //
 *  1, 1,  //
 *  1, 0,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_two_compressed_data[] = {0xE8, 0xB8};
constexpr CompressedImage m_icon_two_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_two_compressed_data};

/**
 * @brief The compressed data of m_icon_three.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  0, 1,  //
 *  1, 1,  //
 *  0, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_three_compressed_data[] = {0xA8, 0xF8};
constexpr CompressedImage m_icon_three_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_three_compressed_data};

/**
 * @brief The compressed data of m_icon_four.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 0,  //
 *  1, 0,  //
 *  1, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_four_compressed_data[] = {0x38, 0xE0};
constexpr CompressedImage m_icon_four_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_four_compressed_data};

/**
 * @brief The compressed data of m_icon_five.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 0,  //
 *  1, 1,  //
 *  0, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_five_compressed_data[] = {0xB8, 0xE8};
constexpr CompressedImage m_icon_five_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_five_compressed_data};

/**
 * @brief The compressed data of m_icon_six.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 0,  //
 *  1, 1,  //
 *  1, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_six_compressed_data[] = {0xF8, 0xE8};
constexpr CompressedImage m_icon_six_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_six_compressed_data};

/**
 * @brief The compressed data of m_icon_seven.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_seven_compressed_data[] = {0x08, 0xF8};
constexpr CompressedImage m_icon_seven_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_seven_compressed_data};

/**
 * @brief The compressed data of m_icon_eight.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 1,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_eight_compressed_data[] = {0xD8, 0xD8};
constexpr CompressedImage m_icon_eight_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_eight_compressed_data};
constexpr uint8_t m_icon_nine[NUM_WIDTH * NUM_HEIGHT] = {

};

/**
 * @brief The compressed data of m_icon_nine.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  1, 1,  //
 *  1, 1,  //
 *  1, 1,  //
 *  0, 1,  //
 *  1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_nine_compressed_data[] = {0xB8, 0xF8};
constexpr CompressedImage m_icon_nine_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_nine_compressed_data};

constexpr CompressedImage m_num_icon_compressed[10] = {
    m_icon_zero_compressed,  m_icon_one_compressed,   m_icon_two_compressed,
    m_icon_three_compressed, m_icon_four_compressed,  m_icon_five_compressed,
    m_icon_six_compressed,   m_icon_seven_compressed, m_icon_eight_compressed,
    m_icon_nine_compressed};

/**
 * @brief The compressed data of m_icon_important.
 *
 * The original data is:
 *
 *  ```
 *  0, 0,  //
 *  0, 0,  //
 *  0, 0,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 1,  //
 *  0, 0,  //
 *  0, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_important_compressed_data[] = {0x00, 0xB8};
constexpr CompressedImage m_icon_important_compressed = {
    .width = NUM_WIDTH,
    .height = NUM_HEIGHT,
    .data = m_icon_important_compressed_data};

/**
 * @brief The compressed data of m_icon_status_overview_heart.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0,  //
 *  0, 1, 0, 1,  //
 *  0, 1, 1, 1,  //
 *  0, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_status_overview_heart_compressed_data[] = {0x00, 0x06,
                                                                    0x0C, 0x06};
constexpr CompressedImage m_icon_status_overview_heart_compressed = {
    .width = FOOD_HEART_OVERVIEW_ICON_WIDTH,
    .height = FOOD_HEART_OVERVIEW_ICON_HEIGHT,
    .data = m_icon_status_overview_heart_compressed_data};

/**
 * @brief The compressed data of m_icon_status_overview_food.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0,  //
 *  0, 0, 1, 0,  //
 *  0, 1, 0, 1,  //
 *  0, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */
constexpr uint8_t m_icon_status_overview_food_compressed_data[] = {0x00, 0x04,
                                                                   0x0A, 0x04};
constexpr CompressedImage m_icon_status_overview_food_compressed = {
    .width = FOOD_HEART_OVERVIEW_ICON_WIDTH,
    .height = FOOD_HEART_OVERVIEW_ICON_HEIGHT,
    .data = m_icon_status_overview_food_compressed_data};

/**
 * @brief The compressed data of m_icon_hospital_compressed_data.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 1, 1, 1, 1, 1, 1, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 0, 1, 1, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_icon_hospital_compressed_data[] = {0x00, 0x18, 0x18, 0x7E,
                                                       0x7E, 0x18, 0x18, 0x00};
constexpr CompressedImage m_icon_hospital_compressed = {
    .width = HOSPITAL_WIDTH,
    .height = HOSPITAL_HEIGHT,
    .data = m_icon_hospital_compressed_data};

/**
 * @brief The compressed data of m_battle_icon.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 1, 1,  //
 *  0, 0, 0, 0, 1, 0, 1,  //
 *  0, 1, 0, 1, 0, 1, 0,  //
 *  0, 1, 1, 0, 1, 0, 0,  //
 *  0, 0, 1, 1, 0, 0, 0,  //
 *  0, 1, 0, 1, 1, 0, 0,  //
 *  1, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_battle_icon_compressed_data[] = {0x80, 0x58, 0x30, 0x68,
                                                     0x54, 0x0A, 0x06};
constexpr CompressedImage m_battle_icon_compressed = {
    .width = BATTLE_WIDTH,
    .height = BATTLE_HEIGHT,
    .data = m_battle_icon_compressed_data};

/**
 * @brief The compressed data of m_training_icon.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 1, 0, 0, 0, 1, 0,  //
 *  1, 1, 0, 0, 0, 1, 1,  //
 *  1, 1, 1, 1, 1, 1, 1,  //
 *  1, 1, 0, 0, 0, 1, 1,  //
 *  0, 1, 0, 0, 0, 1, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_training_icon_compressed_data[] = {0x1C, 0x3E, 0x08, 0x08,
                                                       0x08, 0x3E, 0x1C};
constexpr CompressedImage m_training_icon_compressed = {
    .width = TRAINING_WIDTH,
    .height = TRAINING_HEIGHT,
    .data = m_training_icon_compressed_data};

/**
 * @brief The compressed data of m_YN_icon.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
 *  1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, //
 *  1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, //
 *  1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, //
 *  1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_YN_icon_compressed_data[] = {
    0x3C, 0x08, 0x10, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x30, 0x1C};
constexpr CompressedImage m_YN_icon_compressed = {
    .width = YN_WIDTH, .height = YN_HEIGHT, .data = m_YN_icon_compressed_data};

/**
 * @brief The compressed data of m_YN_select_cursor_left.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  0, 0, 0, 0,  //
 *  1, 1, 1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_YN_select_cursor_left_compressed_data[] = {0x80, 0x80, 0x80,
                                                               0x80};
constexpr CompressedImage m_YN_select_cursor_left_compressed = {
    .width = YN_SELECT_LEFT_WIDTH,
    .height = YN_SELECT_LEFT_HEIGHT,
    .data = m_YN_select_cursor_left_compressed_data};

/**
 * @brief The compressed data of m_YN_select_cursor_right.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  0, 0, 0,  //
 *  1, 1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_YN_select_cursor_right_compressed_data[] = {0x80, 0x80,
                                                                0x80};
constexpr CompressedImage m_YN_select_cursor_right_compressed = {
    .width = YN_SELECT_RIGHT_WIDTH,
    .height = YN_SELECT_RIGHT_HEIGHT,
    .data = m_YN_select_cursor_right_compressed_data};

}  // namespace menu_icon

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#ifdef SIMU
constexpr uint8_t m_example_of_compress[] = {
    1, 1, 1, 1, 1, 1, 1, 1,  //
    0, 1, 1, 1, 1, 1, 1, 1,  //
    0, 0, 1, 1, 1, 1, 1, 1,  //
    0, 0, 0, 1, 1, 1, 1, 1,  //
    0, 0, 0, 0, 1, 1, 1, 1,  //
    0, 0, 0, 0, 0, 1, 1, 1,  //
    0, 0, 0, 0, 0, 0, 1, 1,  //
    0, 0, 0, 0, 0, 0, 0, 1,  //
};
#endif

/*---------- */

// template
/**
 * @brief The compressed data of .
 *
 * The original data is:
 *
 *  ```
 *
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

// constexpr uint8_t _compressed_data[] = {};
// constexpr CompressedImage _compressed = {
//     .width =, .height =, .data = _compressed_data};
