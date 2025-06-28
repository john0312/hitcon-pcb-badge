//#define SIMU  // open if needed


#ifdef SIMU
#define constexpr const
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
#define SLEEP_US 500000
typedef unsigned char uint8_t;

#include <unistd.h>

#include <iostream>

#include "screens.h"

using hitcon::app::tama::components::m_cat_idle1;
using hitcon::app::tama::components::m_cat_idle2;
using hitcon::app::tama::components::m_dog_idle1;
using hitcon::app::tama::components::m_dog_idle2;
using hitcon::app::tama::components::m_select_cursor;
using hitcon::app::tama::components::m_select_print_all_character;
using hitcon::app::tama::egg_icon::m_egg_0_percent_up;
using hitcon::app::tama::egg_icon::m_egg_25_percent_up;
using hitcon::app::tama::egg_icon::m_egg_50_percent_up;
using hitcon::app::tama::egg_icon::m_egg_75_percent_up;
using hitcon::app::tama::egg_icon::m_egg_hatch_shinning1;
using hitcon::app::tama::egg_icon::m_egg_hatch_shinning2;
using hitcon::app::tama::menu_icon::icon_important;
using hitcon::app::tama::menu_icon::num_icon;

/*--- simulation relative start---*/
/**
 * @brief The function to show the full screen in terminal. Will only use in
 * simulation.
 *
 * @param buf the frame to be displayed
 */
void display_input(const uint8_t* buf) {
  std::cout << "┌";
  for (int x = 0; x < DISPLAY_WIDTH; ++x) std::cout << "──";
  std::cout << "┐\n";

  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    std::cout << "│";
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      uint8_t pixel = buf[y * DISPLAY_WIDTH + x];
      std::cout << (pixel ? "██" : "  ");
      // std::cout << (pixel ? "1" : "0");
    }
    std::cout << "│\n";
  }

  std::cout << "└";
  for (int x = 0; x < DISPLAY_WIDTH; ++x) std::cout << "──";
  std::cout << "┘\n";
}

/**
 * @brief Show frame change in terminal.
 *
 * @param frame_collection A array contain frames
 * @param frame_count The len of frame_collection
 * @param delay_us How long to sleep between frames
 */
void show_anime_with_delay(const uint8_t** frame_collection, int frame_count,
                           int delay_us) {
  for (int i = 0; i < frame_count; ++i) {
    // display_input
    display_input(*(frame_collection + i));
    // delay
    usleep(delay_us);
  }
}
/*--- simulation relative end--- */

/** --- basic definition part start ---*/
#define CAT_IDLE_FRAME_COUNT 2
#define DOG_IDLE_FRAME_COUNT 2
#define SELECT_CHARACTER_FRAME_COUNT 2
#define HATCH_WARNING_REPEAT_SHINE_COUNT 3
#define HATCH_STATUS_FRAME_COUNT 4
#define NEW_SCREEN NULL
#define NUM_AREA_WIDTH 8
#define NUM_AREA_HEIGHT 5
#define EGG_AREA_WIDTH 8
#define EGG_AREA_HEIGHT 8
#define HATCH_START_COUNT 400

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
 * @brief The function of stack const component onto a base layer
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
uint8_t* stack_target_offset(const uint8_t* component, uint8_t* base,
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
 * @param target_num The target number to be displayed, from 0 to 999.
 * @return const uint8_t* The address of the number component.
 */
const uint8_t* get_number_component(int target_num) {
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
      .height = 5,
  };

  component_info digit_100x_component_info = {
      .x_len = 2,
      .y_len = 5,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info digit_10x_component_info = {
      .x_len = 2,
      .y_len = 5,
      .x_offset = 3,
      .y_offset = 0,
  };
  component_info digit_1x_component_info = {
      .x_len = 2,
      .y_len = 5,
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
    base = stack_target_offset(num_icon[digit_1x], NULL,
                               digit_1x_component_info, my_base_info);
    digit_count--;
  }

  // stack number icon at 10x digit
  if (digit_count) {
    base = stack_target_offset(num_icon[digit_10x], base,
                               digit_10x_component_info, my_base_info);
    digit_count--;
  }

  // stack number icon at 100x digit
  if (digit_count) {
    base = stack_target_offset(num_icon[digit_100x], base,
                               digit_100x_component_info, my_base_info);
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
 * @return const uint8_t* The address of the warning component.
 */
const uint8_t* get_warning_component() {
  // similar to get_number_component, but only return a warning icon
  base_info my_base_info = {
      .width = 8,
      .height = 5,
  };
  component_info warning_component_info_1 = {
      .x_len = 2,
      .y_len = 5,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info warning_component_info_2 = {
      .x_len = 2,
      .y_len = 5,
      .x_offset = 3,
      .y_offset = 0,
  };
  component_info warning_component_info_3 = {
      .x_len = 2,
      .y_len = 5,
      .x_offset = 6,
      .y_offset = 0,
  };
  // stack warning icon
  uint8_t* base = stack_target_offset(icon_important, NULL,
                                      warning_component_info_1, my_base_info);
  base = stack_target_offset(icon_important, base, warning_component_info_2,
                             my_base_info);
  base = stack_target_offset(icon_important, base, warning_component_info_3,
                             my_base_info);
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
 * @param percentage The percentage of hatching, from 0 to 100.
 * @return const uint8_t* The address of the egg component.
 */
const uint8_t* get_egg_component(int percentage) {
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
    base = stack_target_offset(m_egg_0_percent_up, NULL, egg_component_info,
                               my_base_info);
  } else if (percentage <= 50) {
    base = stack_target_offset(m_egg_25_percent_up, NULL, egg_component_info,
                               my_base_info);
  } else if (percentage <= 75) {
    base = stack_target_offset(m_egg_50_percent_up, NULL, egg_component_info,
                               my_base_info);
  } else if (percentage <= 100) {
    base = stack_target_offset(m_egg_75_percent_up, NULL, egg_component_info,
                               my_base_info);
  }

  return base;
}

/** --- component part end---*/

/** --- frame part start ---*/
/**
 * @brief Get a frame of hatch status, including egg component (reflect hatching
 * status) and number component (reflect remaining count).
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
      .y_offset = 3,
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
  base = stack_target_offset(get_egg_component(percentage), NULL,
                             egg_component_info, my_base_info);
  // stack number icon
  base = stack_target_offset(get_number_component(remaining_count), base,
                             num_component_info, my_base_info);

  return base;
}

/**
 * @brief Get the pet born warning frame
 *
 * The warning frame has stack icon_important component and shining icon.
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
      .y_offset = 3,
  };

  uint8_t* base;
  if (frame == 0) {
    base = stack_target_offset(m_egg_hatch_shinning1, NULL, egg_component_info,
                               my_base_info);
    base = stack_target_offset(get_warning_component(), base,
                               warning_component_info, my_base_info);
  } else {
    base = stack_target_offset(m_egg_hatch_shinning2, NULL, egg_component_info,
                               my_base_info);
    base = stack_target_offset(get_warning_component(), base,
                               warning_component_info, my_base_info);
  }

  return base;
}

/** --- frame part end ---*/

/* ------ example part ----- */
// Just examples of how to use

/**
 * @brief The example of cat_idle
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void cat_idle(int repeat_count) {
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

  uint8_t* cat_idle1 = stack_target_offset(
      m_cat_idle1, NEW_SCREEN, cat_idle_component_info, screen_info);
  uint8_t* cat_idle2 = stack_target_offset(
      m_cat_idle2, NEW_SCREEN, cat_idle_component_info, screen_info);
  const uint8_t* cat_idle_frame_all[CAT_IDLE_FRAME_COUNT] = {cat_idle1,
                                                             cat_idle2};
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(cat_idle_frame_all, CAT_IDLE_FRAME_COUNT, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < CAT_IDLE_FRAME_COUNT; ++i) {
    delete[] cat_idle_frame_all[i];
  }
}

/**
 * @brief The example of dog_idle
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void dog_idle(int repeat_count) {
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
  uint8_t* dog_idle1 = stack_target_offset(
      m_dog_idle1, NEW_SCREEN, dog_idle_component_info, screen_info);
  uint8_t* dog_idle2 = stack_target_offset(
      m_dog_idle2, NEW_SCREEN, dog_idle_component_info, screen_info);
  const uint8_t* dog_idle_frame_all[DOG_IDLE_FRAME_COUNT] = {dog_idle1,
                                                             dog_idle2};
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(dog_idle_frame_all, DOG_IDLE_FRAME_COUNT, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < DOG_IDLE_FRAME_COUNT; ++i) {
    delete[] dog_idle_frame_all[i];
  }
}

/**
 * @brief The example of select_character
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void select_character(int repeat_count) {
  component_info select_left_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  component_info select_right_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 8,
      .y_offset = 0,
  };

  component_info select_print_all_character_component_info = {
      .x_len = 16,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  uint8_t* select_print_all_character1 = stack_target_offset(
      m_select_print_all_character, NEW_SCREEN,
      select_print_all_character_component_info, screen_info);
  uint8_t* select_left =
      stack_target_offset(m_select_cursor, select_print_all_character1,
                          select_left_component_info, screen_info);

  uint8_t* select_print_all_character2 = stack_target_offset(
      m_select_print_all_character, NEW_SCREEN,
      select_print_all_character_component_info, screen_info);
  uint8_t* select_right =
      stack_target_offset(m_select_cursor, select_print_all_character2,
                          select_right_component_info, screen_info);
  const uint8_t* select_character_frame_all[SELECT_CHARACTER_FRAME_COUNT] = {
      select_left, select_right};

  // show animation
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(select_character_frame_all,
                          SELECT_CHARACTER_FRAME_COUNT, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < SELECT_CHARACTER_FRAME_COUNT; ++i) {
    delete[] select_character_frame_all[i];
  }
}

/**
 * @brief The example of egg hatch
 *
 * The example counts down from 390 to 0, and shows the different hatching
 * scene.
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void egg_hatch(int repeat_count) {
  component_info component_info = {
      .x_len = 16,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  // demo case:
  // (remaining_count)
  // 390 > 290 > 190 > 90 > (shine1, shine2)x repeat_shine_count

  // add to frame_collection
  const uint8_t* frame_collection[HATCH_STATUS_FRAME_COUNT +
                                  2 * HATCH_WARNING_REPEAT_SHINE_COUNT];
  int remaining_count = 390;
  for (int j = 0; j < HATCH_STATUS_FRAME_COUNT; ++j) {
    frame_collection[j] = get_hatch_status_frame(remaining_count);
    remaining_count -= 100;
  }
  for (int j = 0; j < HATCH_WARNING_REPEAT_SHINE_COUNT; ++j) {
    frame_collection[HATCH_STATUS_FRAME_COUNT + j * 2] =
        get_hatch_born_warning_frame(0);
    frame_collection[HATCH_STATUS_FRAME_COUNT + j * 2 + 1] =
        get_hatch_born_warning_frame(1);
  }

  // show animation
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(
        frame_collection,
        HATCH_STATUS_FRAME_COUNT + 2 * HATCH_WARNING_REPEAT_SHINE_COUNT,
        SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0;
       i < HATCH_STATUS_FRAME_COUNT + 2 * HATCH_WARNING_REPEAT_SHINE_COUNT;
       ++i) {
    delete[] frame_collection[i];
  }
}

/**
 * @brief The example of number test
 *
 * This function will show diffent digits examples of number icon, like
 * 1, 23, 456, 789, 100.
 *
 * Note: 23 can't fill like 023, or it will be recognized as octal format
 * number.
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void num_test(int repeat_count) {
  const int frame_count = 5;
  component_info num_area_component_info = {
      .x_len = 8,
      .y_len = 5,
      .x_offset = 0,
      .y_offset = 0,
  };
  base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  const uint8_t* num_component_all[frame_count] = {
      get_number_component(1),   get_number_component(23),
      get_number_component(456), get_number_component(789),
      get_number_component(100),
  };

  // create screens
  const uint8_t* frame_all[frame_count];
  for (int i = 0; i < frame_count; ++i) {
    frame_all[i] = stack_target_offset(num_component_all[i], NEW_SCREEN,
                                       num_area_component_info, screen_info);
  }

  // show animation
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(frame_all, frame_count, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < frame_count; ++i) {
    delete[] frame_all[i];
  }
  for (int i = 0; i < frame_count; ++i) {
    delete[] num_component_all[i];
  }
}

int main() {
  int repeat_count = 3;
  int repeat_once = 1;

  // cat idle demo
  std::cout << "Cat Idle Demo:\n";
  cat_idle(repeat_count);

  // dog idle demo
  std::cout << "Dog Idle Demo:\n";
  dog_idle(repeat_count);

  // selection demo
  std::cout << "Select Character Demo:\n";
  select_character(repeat_count);

  // number icon demo
  std::cout << "Number Icon Demo:\n";
  num_test(repeat_once);

  // egg hatch demo
  std::cout << "Egg Hatch Demo:\n";
  egg_hatch(repeat_once);

  return 0;
}

#endif
