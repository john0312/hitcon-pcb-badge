#ifndef SCREENS_H
#define SCREENS_H
/// -- -- -- --

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
