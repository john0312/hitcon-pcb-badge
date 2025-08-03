#include "TamaAppFrame.h"

// ------

/** --- basic definition part start ---*/

/**
 * @brief The function of stack component/frame onto a base layer
 * Use this function if you need to use decompress_component.
 *
 * @param component the component to be stacked on the top. Only accept const,
 * because components are const in screens.h
 * The component will be freed after use.
 * @param base the base to be stacked on. Can pass the base you want to modify,
 * or pass NULL create from a new one
 * @param comp_info The info of the input component.
 * @param bs_info The info of the base. Must be valid, but only used when
 * base is NULL.
 * @param eliminate If true, the component will be eliminated from the base.
 * @return uint8_t* : The address of the base. For multi-layer stack.
 */
void stack_component(uint8_t* component, uint8_t* base,
                     component_info comp_info, base_info bs_info,
                     bool eliminate) {
  // edge check
  if (comp_info.x_offset + comp_info.x_len > bs_info.width ||
      comp_info.y_offset + comp_info.y_len > bs_info.height) {
#ifdef SIMU
    std::cerr << "Error: Component exceeds base dimensions." << std::endl;
#endif
  }

  // stack new component onto base
  for (int y = 0; y < comp_info.y_len; ++y) {
    for (int x = 0; x < comp_info.x_len; ++x) {
      int base_index =
          (comp_info.y_offset + y) * bs_info.width + (comp_info.x_offset + x);
      int component_index = y * comp_info.x_len + x;
      int bit_status;
      if (eliminate) {
        if (base[base_index]) {
          // eliminate the component from base
          bit_status = base[base_index] & ~component[component_index];
        } else {
          /* This provide more attach effect, can delete if not good*/
          // if base is empty, just use the component
          bit_status = component[component_index];
        }
      } else {
        // stack the component onto base
        bit_status = component[component_index] | base[base_index];
      }
      base[base_index] = bit_status;
    }
  }
}
/**
 * @brief The function of stack const component onto a base layer
 *
 * @param component the component to be stacked on the top. Only accept const,
 * because components are const in screens.h
 * component should be const, will not be free.
 * @param base the base to be stacked on. Can pass the base you want to modify,
 * or pass NULL create from a new one
 * @param comp_info The info of the input component.
 * @param bs_info The info of the base. Must be valid, but only used when base
 * is NULL.
 */
void stack_const_component(const uint8_t* component, uint8_t* base,
                           component_info comp_info, base_info bs_info) {
  // edge check
  if (comp_info.x_offset + comp_info.x_len > bs_info.width ||
      comp_info.y_offset + comp_info.y_len > bs_info.height) {
#ifdef SIMU
    std::cerr << "Error: Component exceeds base dimensions." << std::endl;
#endif
  }

  // stack new component onto base
  for (int y = 0; y < comp_info.y_len; ++y) {
    for (int x = 0; x < comp_info.x_len; ++x) {
      int base_index =
          (comp_info.y_offset + y) * bs_info.width + (comp_info.x_offset + x);
      int component_index = y * comp_info.x_len + x;
      int bit_status = component[component_index] | base[base_index];
      base[base_index] = bit_status;
    }
  }
}
/** --- basic definition part end ---*/
/** --- component part end---*/
void get_scoring_frame(int ok_qty, int fail_qty, uint8_t* base) {
  const component_info icon_component_info = {
      .x_len = 5,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  const base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  component_info score_icon_info_base = {
      .x_len = 1,
      .y_len = 3,
      .x_offset = 6,
      .y_offset = 1,
  };

  // icon
  const CompressedImage* target = &m_scoring_page_icon_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, icon_component_info, screen_info);

  // score
  for (int i = 0; i < ok_qty; ++i) {
    component_info score_icon_info_copy = score_icon_info_base;
    score_icon_info_copy.x_offset += i * 2;  // offset for each icon
    const CompressedImage* target = &m_score_icon_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, score_icon_info_copy,
                    screen_info);
  }
  for (int i = 0; i < fail_qty; ++i) {
    component_info score_icon_info_copy = score_icon_info_base;
    score_icon_info_copy.y_offset = 5;       // offset for fail icon
    score_icon_info_copy.x_offset += i * 2;  // offset for each icon
    const CompressedImage* target = &m_score_icon_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, score_icon_info_copy,
                    screen_info);
  }
}

void get_end_frame(uint8_t* base) {
  const component_info full_frame_component_info = {
      .x_len = 16,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  const base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  const CompressedImage* target = &m_battle_training_end_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, full_frame_component_info,
                  screen_info);
}
/** --- frame part end ---*/
