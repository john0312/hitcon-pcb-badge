#ifndef SCREENS_H
#define SCREENS_H

#include "simu_setting.h"
/// -- -- -- --

#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
#define COMMON_HEIGHT 8
#define IDLE_PET_WIDTH 8
#define SELECT_WIDTH 8
#define NUM_WIDTH 2
#define EGG_WIDTH 8
#define FOOD_HEART_OVERVIEW_ICON_WIDTH 4
#define FOOD_HEART_OVERVIEW_ICON_HEIGHT 4
#define WEAK_PET_WIDTH 8
#define WEAK_PET_PARTICLE_WIDTH 8
#define HOSPITAL_WIDTH 8
#define BATTLE_WIDTH 7
#define TRAINING_WIDTH 7
#define YN_WIDTH 16
#define YN_SELECT_LEFT_WIDTH 4
#define YN_SELECT_RIGHT_WIDTH 3

// m_xxx = material xxx

enum {
  FRAME_1,
  FRAME_2,
};

enum {
  PET_TYPE_DOG,
  PET_TYPE_CAT,
  OTHER_TYPE_TRAINING_FACILITY,
};

enum {
  BATTLE,
  TRAINING,
};

enum {
  LEFT,
  RIGHT,
};

enum {
  WIN,
  LOSE,
};

enum {
  PLAYER,
  ENEMY,
  NONE,
};

enum {
  cookie_100,
  cookie_50,
  cookie_30,
  cookie_0,
  ate_frame0,
  ate_frame1,
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
 * @brief The compressed data of cat_battle_result.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 0, 0, 1, 0, 1,  //
 *  0, 0, 1, 0, 1, 1, 1,  //
 *  0, 1, 0, 1, 1, 1, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_cat_battle_result_compressed_data[] = {
    0x20, 0xA0, 0x40, 0x80, 0xE0, 0xC0, 0xE0};
constexpr CompressedImage m_cat_battle_result_compressed = {
    .width = 7, .height = 8, .data = m_cat_battle_result_compressed_data};

/**
 * @brief The compressed data of battle_result_win_effect.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,  //
 *  0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,  //
 *  0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,  //
 *  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_battle_result_win_effect_compressed_data[] = {
    0x00, 0x02, 0x04, 0x08, 0x01, 0x02, 0x04, 0x00,
    0x03, 0x00, 0x04, 0x02, 0x01, 0x08, 0x04, 0x02};
constexpr CompressedImage m_battle_result_win_effect_compressed = {
    .width = 16,
    .height = 8,
    .data = m_battle_result_win_effect_compressed_data};

/**
 * @brief The compressed data of battle_result_lost_effect.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_battle_result_lose_effect_compressed_data[] = {
    0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00,
    0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00};
constexpr CompressedImage m_battle_result_lose_effect_compressed = {
    .width = 16,
    .height = 8,
    .data = m_battle_result_lose_effect_compressed_data};

/**
 * @brief The compressed data of m_training_facility_enemy.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 1, 0, 0, 0, 0,  //
 *  0, 1, 0, 1, 0, 1, 0,  //
 *  0, 1, 0, 1, 1, 0, 1,  //
 *  0, 0, 1, 0, 1, 0, 1,  //
 *  0, 0, 1, 0, 0, 1, 0,  //
 *  0, 1, 1, 1, 0, 1, 0,  //
 *  0, 0, 0, 0, 1, 1, 1,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_training_facility_enemy_compressed_data[] = {
    0x00, 0x26, 0x39, 0x26, 0x4C, 0x72, 0x4C};
constexpr CompressedImage m_training_facility_enemy_compressed = {
    .width = 7, .height = 8, .data = m_training_facility_enemy_compressed_data};

/**
 * @brief The compressed data of m_hit_player_effect.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  1, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 1, 0, 0, 1, 0, 0, 0,  //
 *  0, 0, 1, 0, 0, 1, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 1, 0,  //
 *  0, 0, 0, 0, 1, 0, 0, 1,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_hit_player_effect_compressed_data[] = {
    0x08, 0x10, 0x20, 0x48, 0x90, 0x20, 0x40, 0x80};
constexpr CompressedImage m_hit_player_effect_compressed = {
    .width = 8, .height = 8, .data = m_hit_player_effect_compressed_data};

/**
 * @brief The compressed data of m_hit_enemy_effect.
 *
 * The original data is:
 *
 *  ```
 *  1, 0, 0, 1, 0, 0, 0, 0,  //
 *  0, 1, 0, 0, 1, 0, 0, 0,  //
 *  0, 0, 1, 0, 0, 1, 0, 0,  //
 *  0, 0, 0, 1, 0, 0, 1, 0,  //
 *  0, 0, 0, 0, 1, 0, 0, 1,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_hit_enemy_effect_compressed_data[] = {
    0x01, 0x02, 0x04, 0x09, 0x12, 0x04, 0x08, 0x10};
constexpr CompressedImage m_hit_enemy_effect_compressed = {
    .width = 8, .height = 8, .data = m_hit_enemy_effect_compressed_data};

}  // namespace components

namespace menu_icon {
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
    .height = COMMON_HEIGHT,
    .data = m_training_icon_compressed_data};

/**
 * @brief The compressed data of m_lv_word_icon (level).
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  1, 1, 1, 1, 1, 1, 1,  //
 *  0, 0, 0, 0, 0, 0, 0,  //
 *  1, 0, 0, 0, 1, 0, 1,  //
 *  1, 0, 0, 0, 1, 0, 1,  //
 *  1, 0, 0, 0, 1, 0, 1,  //
 *  1, 0, 0, 0, 1, 0, 1,  //
 *  1, 1, 1, 0, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_lv_word_icon_compressed_data[] = {0xFA, 0x82, 0x82, 0x02,
                                                      0x7A, 0x82, 0x7A};
constexpr CompressedImage m_lv_word_icon_compressed = {
    .width = 7,
    .height = COMMON_HEIGHT,
    .data = m_lv_word_icon_compressed_data};

/**
 * @brief The compressed data of m_battle_training_end.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0,  //
 *  0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0,  //
 *  0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0,  //
 *  0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,  //
 *  0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0,  //
 *  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_battle_training_end_compressed_data[] = {
    0x00, 0x00, 0x7C, 0x54, 0x54, 0x00, 0x7C, 0x18,
    0x30, 0x7C, 0x00, 0x7C, 0x44, 0x44, 0x38, 0x00};
constexpr CompressedImage m_battle_training_end_compressed = {
    .width = 16, .height = 8, .data = m_battle_training_end_compressed_data};

/**
 * @brief The compressed data of m_scoring_page_icon.
 *
 * The original data is:
 *
 *  ```
 *  0, 0, 0, 0, 0,  //
 *  0, 1, 0, 1, 0,  //
 *  0, 1, 0, 1, 0,  //
 *  0, 0, 1, 0, 0,  //
 *  0, 0, 0, 0, 0,  //
 *  0, 1, 0, 1, 0,  //
 *  0, 0, 1, 0, 0,  //
 *  0, 1, 0, 1, 0,  //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_scoring_page_icon_compressed_data[] = {0x00, 0xA6, 0x48,
                                                           0xA6, 0x00};
constexpr CompressedImage m_scoring_page_icon_compressed = {
    .width = 5, .height = 8, .data = m_scoring_page_icon_compressed_data};

/**
 * @brief The compressed data of m_score_icon.
 *
 * The original data is:
 *
 *  ```
 *  1, //
 *  1, //
 *  1, //
 *  ```
 *
 * Notice: the compressed data is not directly mapping to the original data.
 * It packed the bits in a specific way.
 *
 */

constexpr uint8_t m_score_icon_compressed_data[] = {
    0x07,
};
constexpr CompressedImage m_score_icon_compressed = {
    .width = 1, .height = 3, .data = m_score_icon_compressed_data};

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

#endif
