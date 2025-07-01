#include "TamaAppUtils.h"

// -------
using hitcon::app::tama::components::m_cat_idle1_compressed;
using hitcon::app::tama::components::m_cat_idle2_compressed;
using hitcon::app::tama::components::m_cat_weak_compressed;
using hitcon::app::tama::components::m_dog_idle1_compressed;
using hitcon::app::tama::components::m_dog_idle2_compressed;
using hitcon::app::tama::components::m_dog_weak_compressed;
using hitcon::app::tama::components::m_select_cursor_compressed;
using hitcon::app::tama::components::m_select_print_all_character_compressed;
using hitcon::app::tama::components::m_weak_particle_1_compressed;
using hitcon::app::tama::components::m_weak_particle_2_compressed;
using hitcon::app::tama::egg_icon::m_egg_0_percent_up_compressed;
using hitcon::app::tama::egg_icon::m_egg_25_percent_up_compressed;
using hitcon::app::tama::egg_icon::m_egg_50_percent_up_compressed;
using hitcon::app::tama::egg_icon::m_egg_75_percent_up_compressed;
using hitcon::app::tama::egg_icon::m_egg_hatch_shinning1_compressed;
using hitcon::app::tama::egg_icon::m_egg_hatch_shinning2_compressed;
using hitcon::app::tama::menu_icon::m_icon_important_compressed;
using hitcon::app::tama::menu_icon::m_icon_status_overview_food_compressed;
using hitcon::app::tama::menu_icon::m_icon_status_overview_heart_compressed;
using hitcon::app::tama::menu_icon::m_num_icon_compressed;
// ------

/** --- basic definition part start ---*/
#define CAT_IDLE_FRAME_COUNT 2
#define DOG_IDLE_FRAME_COUNT 2
#define SELECT_CHARACTER_FRAME_COUNT 2
#define HATCH_WARNING_REPEAT_SHINE_COUNT 3
#define HATCH_STATUS_FRAME_COUNT 4
#define NEW_SCREEN NULL
#define NUM_AREA_WIDTH 8
#define NUM_AREA_HEIGHT 8
#define EGG_AREA_WIDTH 8
#define EGG_AREA_HEIGHT 8
#define HATCH_START_COUNT 400
#define FOOD_HEART_OVERVIEW_COMPONENT_WIDTH 8
#define FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT 4

typedef struct COMPONENT_INFO {
  int x_offset;
  int y_offset;
  int x_len;
  int y_len;
} component_info;

typedef struct BASE_INFO {
  int width;
  int height;
} base_info;

/**
 * @brief The function of stack component/frame onto a base layer
 *
 * @param component the component to be stacked on the top. Only accept const,
 * because components are const in screens.h
 * @param base the base to be stacked on. Can pass the base you want to modify,
 * or pass NULL create from a new one
 * @param comp_info The info of the input component.
 * @param bs_info The info of the base. Must be valid, but only used when
 * base is NULL.
 * @return uint8_t* : The address of the base. For multi-layer stack.
 */
uint8_t* stack_component(uint8_t* component, uint8_t* base,
                         component_info comp_info, base_info bs_info) {
  // use a new base if input is empty
  if (base == NULL) {
    if (bs_info.width <= 0 || bs_info.height <= 0) {
      std::cerr << "Error: Invalid base dimensions." << std::endl;
      return nullptr;
    }
    // create a new memory at heap and fill with 0
    base = new uint8_t[bs_info.width * bs_info.height]();
    if (base == nullptr) {
      std::cerr << "Error: Memory allocation failed." << std::endl;
      return nullptr;
    }
  }

  // edge check
  if (comp_info.x_offset + comp_info.x_len > bs_info.width ||
      comp_info.y_offset + comp_info.y_len > bs_info.height) {
    std::cerr << "Error: Component exceeds base dimensions." << std::endl;
    return base;
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

  // free the input component
  delete[] component;

  return base;
}
/**
 * @brief The function of stack const component onto a base layer
 *
 * @param component the component to be stacked on the top. Only accept const,
 * because components are const in screens.h
 * @param base the base to be stacked on. Can pass the base you want to modify,
 * or pass NULL create from a new one
 * @param comp_info The info of the input component.
 * @param bs_info The info of the base. Must be valid, but only used when base
 * is NULL.
 * @param free_input_component The input component need to free or not after
 * using.
 */
uint8_t* stack_target_offset(const uint8_t* component, uint8_t* base,
                             component_info comp_info, base_info bs_info,
                             bool free_input_component) {
  // use a new base if input is empty
  if (base == NULL) {
    if (bs_info.width <= 0 || bs_info.height <= 0) {
      std::cerr << "Error: Invalid base dimensions." << std::endl;
      return nullptr;
    }
    // create a new memory at heap and fill with 0
    base = new uint8_t[bs_info.width * bs_info.height]();
    if (base == nullptr) {
      std::cerr << "Error: Memory allocation failed." << std::endl;
      return nullptr;
    }
  }

  // edge check
  if (comp_info.x_offset + comp_info.x_len > bs_info.width ||
      comp_info.y_offset + comp_info.y_len > bs_info.height) {
    std::cerr << "Error: Component exceeds base dimensions." << std::endl;
    return base;
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

  if (free_input_component) {
    // free the input component if needed
    delete[] component;
  }

  return base;
}
/** --- basic definition part end ---*/

/** --- component part start ---*/
/**
 * @brief Get the number component based on the target number.
 *
 * The number component will be a stack of three digits, each digit is 2x5
 * pixels. The maximum number is 999, and the minimum is 0.
 *
 * Must have to free the returned pointer after use.
 *
 * @param target_num The target number to be displayed, from 0 to 999.
 * @return uint8_t* The address of the number component.
 */
uint8_t* get_number_component(int target_num) {
  // check boundary of input
  if (target_num > 999) {
    target_num = 999;
  }
  if (target_num < 0) {
    target_num = 0;
  }

  // how many digits need to be displayed
  int digit_count = 1;
  if (target_num >= 10) {
    digit_count++;
  }
  if (target_num >= 100) {
    digit_count++;
  }

  // parse digits
  int digit_100x = target_num / 100;
  target_num = target_num % 100;
  int digit_10x = target_num / 10;
  int digit_1x = target_num % 10;

  base_info my_base_info = {
      .width = 8,
      .height = 8,
  };

  component_info digit_100x_component_info = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info digit_10x_component_info = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 3,
      .y_offset = 0,
  };
  component_info digit_1x_component_info = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 6,
      .y_offset = 0,
  };

  /**
   * v     v     v     offset
   * 0 1 2 3 4 5 6 7
   *     x     x       empty space
   */

  uint8_t* base;
  // stack number icon at 1x digit
  if (digit_count) {
    base = stack_target_offset(
        decompress_component(&m_num_icon_compressed[digit_1x]), NULL,
        digit_1x_component_info, my_base_info, false);
    digit_count--;
  }

  // stack number icon at 10x digit
  if (digit_count) {
    base = stack_target_offset(
        decompress_component(&m_num_icon_compressed[digit_10x]), base,
        digit_10x_component_info, my_base_info, false);
    digit_count--;
  }

  // stack number icon at 100x digit
  if (digit_count) {
    base = stack_target_offset(
        decompress_component(&m_num_icon_compressed[digit_100x]), base,
        digit_100x_component_info, my_base_info, false);
    digit_count--;
  }

  return base;
}

/**
 * @brief Get the warning component, which is a stack of three warning icons.
 *
 * The warning component is used to indicate that the pet is born and needs
 * attention.
 *
 * Must have to free the returned pointer after use.
 *
 * @return uint8_t* The address of the warning component.
 */
uint8_t* get_warning_component() {
  // similar to get_number_component, but only return a warning icon
  base_info my_base_info = {
      .width = 8,
      .height = 8,
  };
  component_info warning_component_info_1 = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info warning_component_info_2 = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 3,
      .y_offset = 0,
  };
  component_info warning_component_info_3 = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 6,
      .y_offset = 0,
  };
  // stack warning icon
  uint8_t* base =
      stack_target_offset(decompress_component(&m_icon_important_compressed),
                          NULL, warning_component_info_1, my_base_info, false);
  base =
      stack_target_offset(decompress_component(&m_icon_important_compressed),
                          base, warning_component_info_2, my_base_info, false);
  base =
      stack_target_offset(decompress_component(&m_icon_important_compressed),
                          base, warning_component_info_3, my_base_info, false);
  // return the base with warning icon
  return base;
}

/**
 * @brief Get the egg component based on the hatching percentage.
 *
 * The egg component will be one of the four stages:
 * 0% (egg), 25% (egg with cracks), 50% (egg with more cracks),
 * 75% (egg with even more cracks).
 *
 * Must have to free the returned pointer after use.
 *
 * @param percentage The percentage of hatching, from 0 to 100.
 * @return const uint8_t* The address of the egg component.
 */
uint8_t* get_egg_component(int percentage) {
  // check boundary of input
  if (percentage > 100) {
    percentage = 100;
  }
  if (percentage < 0) {
    percentage = 0;
  }

  base_info my_base_info = {
      .width = EGG_AREA_WIDTH,
      .height = EGG_AREA_HEIGHT,
  };

  component_info egg_component_info = {
      .x_len = EGG_AREA_WIDTH,
      .y_len = EGG_AREA_HEIGHT,
      .x_offset = 0,
      .y_offset = 0,
  };

  uint8_t* base;
  if (percentage <= 25) {
    base = stack_target_offset(
        decompress_component(&m_egg_0_percent_up_compressed), NULL,
        egg_component_info, my_base_info, false);
  } else if (percentage <= 50) {
    base = stack_target_offset(
        decompress_component(&m_egg_25_percent_up_compressed), NULL,
        egg_component_info, my_base_info, false);
  } else if (percentage <= 75) {
    base = stack_target_offset(
        decompress_component(&m_egg_50_percent_up_compressed), NULL,
        egg_component_info, my_base_info, false);
  } else if (percentage <= 100) {
    base = stack_target_offset(
        decompress_component(&m_egg_75_percent_up_compressed), NULL,
        egg_component_info, my_base_info, false);
  }

  return base;
}

/**
 * @brief Get the heart overview component object.
 * The component will use at the idle page.
 *
 * @param heart_count The number of hearts to be displayed, from 0 to 3.
 * @return uint8_t* The address of the heart overview component.
 */
uint8_t* get_heart_overview_component(int heart_count) {
  // check boundary of input
  if (heart_count < 0) {
    heart_count = 0;
  }
  if (heart_count > 3) {
    heart_count = 3;
  }

  base_info my_base_info = {
      .width = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .height = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
  };

  component_info heart_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_ICON_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_ICON_HEIGHT,
      .x_offset = 0,
      .y_offset = 0,
  };
  uint8_t* base;
  // icon
  base = stack_target_offset(
      decompress_component(&m_icon_status_overview_heart_compressed), NULL,
      heart_component_info, my_base_info, false);
  if (heart_count < 1) {
    // no food, return the base with icon only
    return base;
  }

  // count
  constexpr uint8_t full_1x1[1 * 1] = {1};
  component_info full_1x1_stack_left_top = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 5,
      .y_offset = 1,
  };
  component_info full_1x1_stack_mid_mid = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 6,
      .y_offset = 2,
  };
  component_info full_1x1_stack_right_bottom = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 7,
      .y_offset = 3,
  };

  base = stack_target_offset(full_1x1, base, full_1x1_stack_left_top,
                             my_base_info, false);
  heart_count -= 1;
  if (heart_count < 1) {
    return base;
  }
  base = stack_target_offset(full_1x1, base, full_1x1_stack_mid_mid,
                             my_base_info, false);
  heart_count -= 1;
  if (heart_count < 1) {
    return base;
  }

  base = stack_target_offset(full_1x1, base, full_1x1_stack_right_bottom,
                             my_base_info, false);
  return base;
}

/**
 * @brief Get the food overview component object.
 * The component will use at the idle page.
 *
 * The component will display the food icon and the number of food.
 * The maximum number of food is 4, and the minimum is 0.
 *
 * @param food_count The number of food to be displayed, from 0 to 4.
 * @return uint8_t* The address of the food overview component.
 */
uint8_t* get_food_overview_component(int food_count) {
  // check boundary of input
  if (food_count < 0) {
    food_count = 0;
  }
  if (food_count > 4) {
    food_count = 4;
  }

  base_info my_base_info = {
      .width = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .height = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
  };

  component_info food_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_ICON_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_ICON_HEIGHT,
      .x_offset = 0,
      .y_offset = 0,
  };

  uint8_t* base;
  // icon
  base = stack_target_offset(
      decompress_component(&m_icon_status_overview_food_compressed), NULL,
      food_component_info, my_base_info, false);
  if (food_count < 1) {
    // no food, return the base with icon only
    return base;
  }

  // count
  constexpr uint8_t full_1x1[1 * 1] = {1};
  component_info full_1x1_stack_left_top = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 5,
      .y_offset = 1,
  };
  component_info full_1x1_stack_right_top = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 7,
      .y_offset = 1,
  };
  component_info full_1x1_stack_left_bottom = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 5,
      .y_offset = 3,
  };
  component_info full_1x1_stack_right_bottom = {
      .x_len = 1,
      .y_len = 1,
      .x_offset = 7,
      .y_offset = 3,
  };

  base = stack_target_offset(full_1x1, base, full_1x1_stack_left_top,
                             my_base_info, false);
  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return base;
  }
  base = stack_target_offset(full_1x1, base, full_1x1_stack_right_top,
                             my_base_info, false);
  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return base;
  }
  base = stack_target_offset(full_1x1, base, full_1x1_stack_left_bottom,
                             my_base_info, false);
  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return base;
  }
  base = stack_target_offset(full_1x1, base, full_1x1_stack_right_bottom,
                             my_base_info, false);

  return base;
}

/** --- component part end---*/

/** --- frame part start ---*/
/**
 * @brief Get a frame of hatch status, including egg component (reflect
 * hatching status) and number component (reflect remaining count).
 *
 * Size is DISPLAY_WIDTH * DISPLAY_HEIGHT.
 */
const uint8_t* get_hatch_status_frame(int remaining_count) {
  // check boundary of input
  if (remaining_count > 999) {
    remaining_count = 999;
  }

  base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  component_info egg_component_info = {
      .x_len = EGG_AREA_WIDTH,
      .y_len = EGG_AREA_HEIGHT,
      .x_offset = 0,
      .y_offset = 0,
  };

  component_info num_component_info = {
      .x_len = NUM_AREA_WIDTH,
      .y_len = NUM_AREA_HEIGHT,
      .x_offset = 8,
      .y_offset = 0,
  };

  // percentage of egg hatching
  int percentage = remaining_count * 100 / HATCH_START_COUNT;
  if (percentage < 0) {
    percentage = 0;
  }
  if (percentage > 100) {
    percentage = 100;
  }

  uint8_t* base;
  /* combine hatch component with num component */
  // stack egg icon
  base = stack_component(get_egg_component(percentage), NULL,
                         egg_component_info, my_base_info);
  // stack number icon
  base = stack_component(get_number_component(remaining_count), base,
                         num_component_info, my_base_info);

  return base;
}

/**
 * @brief Get the pet born warning frame
 *
 * The warning frame has stack m_icon_important component and shining icon.
 *
 * @param frame 0 or 1, to get different frame
 * @return const uint8_t* The address of the frame
 */
const uint8_t* get_hatch_born_warning_frame(int frame) {
  // check boundary of input
  if (frame < 0 || frame > 1) {
    frame = 0;
  }

  base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  component_info egg_component_info = {
      .x_len = EGG_AREA_WIDTH,
      .y_len = EGG_AREA_HEIGHT,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info warning_component_info = {
      .x_len = NUM_AREA_WIDTH,
      .y_len = NUM_AREA_HEIGHT,
      .x_offset = 8,
      .y_offset = 0,
  };

  uint8_t* base;
  if (frame == 0) {
    base = stack_target_offset(
        decompress_component(&m_egg_hatch_shinning1_compressed), NULL,
        egg_component_info, my_base_info, false);
    base = stack_component(get_warning_component(), base,
                           warning_component_info, my_base_info);
  } else {
    base = stack_target_offset(
        decompress_component(&m_egg_hatch_shinning2_compressed), NULL,
        egg_component_info, my_base_info, false);
    base = stack_component(get_warning_component(), base,
                           warning_component_info, my_base_info);
  }

  return base;
}

/**
 * @brief Get the dog idle frame with status overview.
 *
 * The frame will include dog idle component, heart overview component and food
 * overview component.
 *
 * @param frame 0 or 1, to get different frame
 * @param heart_count The number of hearts to be displayed, from 0 to 3.
 * @param food_count The number of food to be displayed, from 0 to 4.
 * @return const uint8_t* The address of the frame
 */
const uint8_t* get_dog_idle_frame_with_status_overview(int frame,
                                                       int heart_count,
                                                       int food_count) {
  // check boundary of input
  if (frame < 0 || frame > 1) {
    frame = 0;
  }

  base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  component_info dog_idle_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  base_info screen_info = {
      .width = 16,
      .height = 8,
  };
  component_info heart_overview_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
      .x_offset = 8,
      .y_offset = 0,
  };
  component_info food_overview_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
      .x_offset = 8,
      .y_offset = 4,
  };
  component_info dog_weak_component_info = dog_idle_component_info;
  component_info weak_particle_component_info = dog_idle_component_info;

  uint8_t* base;
  if (heart_count == 0 || food_count == 0) {
    base = stack_component(decompress_component(&m_dog_weak_compressed),
                           NEW_SCREEN, dog_weak_component_info, screen_info);
    if (frame == IDLE_1) {
      base =
          stack_component(decompress_component(&m_weak_particle_1_compressed),
                          base, weak_particle_component_info, screen_info);
    } else if (frame == IDLE_2) {
      base =
          stack_component(decompress_component(&m_weak_particle_2_compressed),
                          base, weak_particle_component_info, screen_info);
    }
  } else {
    if (frame == IDLE_1) {
      base = stack_component(decompress_component(&m_dog_idle1_compressed),
                             NEW_SCREEN, dog_idle_component_info, screen_info);
    } else if (frame == IDLE_2) {
      base = stack_target_offset(decompress_component(&m_dog_idle2_compressed),
                                 NEW_SCREEN, dog_idle_component_info,
                                 screen_info, false);
    }
  }

  base = stack_component(get_heart_overview_component(heart_count), base,
                         heart_overview_component_info, screen_info);
  base = stack_component(get_food_overview_component(food_count), base,
                         food_overview_component_info, screen_info);

  return base;
}

/**
 * @brief Get the cat idle frame with status overview.
 *
 * The frame will include cat idle component, heart overview component and food
 * overview component.
 *
 * @param frame 0 or 1, to get different frame
 * @param heart_count The number of hearts to be displayed, from 0 to 3.
 * @param food_count The number of food to be displayed, from 0 to 4.
 * @return const uint8_t* The address of the frame
 */
const uint8_t* get_cat_idle_frame_with_status_overview(int frame,
                                                       int heart_count,
                                                       int food_count) {
  // check boundary of input
  if (frame < 0 || frame > 1) {
    frame = 0;
  }

  base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  component_info cat_idle_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  base_info screen_info = {
      .width = 16,
      .height = 8,
  };
  component_info heart_overview_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
      .x_offset = 8,
      .y_offset = 0,
  };
  component_info food_overview_component_info = {
      .x_len = FOOD_HEART_OVERVIEW_COMPONENT_WIDTH,
      .y_len = FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT,
      .x_offset = 8,
      .y_offset = 4,
  };

  component_info cat_weak_component_info = cat_idle_component_info;
  component_info weak_particle_component_info = cat_idle_component_info;

  uint8_t* base;
  if (heart_count == 0 || food_count == 0) {
    base = stack_component(decompress_component(&m_cat_weak_compressed),
                           NEW_SCREEN, cat_weak_component_info, screen_info);
    if (frame == IDLE_1) {
      base =
          stack_component(decompress_component(&m_weak_particle_1_compressed),
                          base, weak_particle_component_info, screen_info);
    } else if (frame == IDLE_2) {
      base =
          stack_component(decompress_component(&m_weak_particle_2_compressed),
                          base, weak_particle_component_info, screen_info);
    }
  } else {
    if (frame == IDLE_1) {
      base = stack_target_offset(decompress_component(&m_cat_idle1_compressed),
                                 NEW_SCREEN, cat_idle_component_info,
                                 screen_info, false);
    } else if (frame == IDLE_2) {
      base = stack_target_offset(decompress_component(&m_cat_idle2_compressed),
                                 NEW_SCREEN, cat_idle_component_info,
                                 screen_info, false);
    }
  }

  base = stack_component(get_heart_overview_component(heart_count), base,
                         heart_overview_component_info, screen_info);
  base = stack_component(get_food_overview_component(food_count), base,
                         food_overview_component_info, screen_info);

  return base;
}

/** --- frame part end ---*/