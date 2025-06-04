#define constexpr const
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
#define SLEEP_US 500000
#define CAT_IDLE_FRAME_COUNT 2
#define DOG_IDLE_FRAME_COUNT 2
#define SELECT_CHARACTER_FRAME_COUNT 2
#define NEW_SCREEN NULL
#define NUM_AREA_WIDTH 8
#define NUM_AREA_HEIGHT 5
typedef unsigned char uint8_t;

#include <unistd.h>

#include <iostream>

#include "screens.h"

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

using hitcon::app::tama::components::m_cat_idle1;
using hitcon::app::tama::components::m_cat_idle2;
using hitcon::app::tama::components::m_dog_idle1;
using hitcon::app::tama::components::m_dog_idle2;
using hitcon::app::tama::components::m_select_cursor;
using hitcon::app::tama::components::m_select_print_all_character;
using hitcon::app::tama::menu_icon::num_icon;

/**
 * @brief The function of stack const component onto a base layer
 *
 * @param component the component to be stacked on the top. Only accept const,
 * because components are const in screens.h
 * @param base the base to be stacked on. Can pass the base you want to modify,
 * or pass NULL create from a new one
 * @param comp_info The info of the input component.
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

/**
 * @brief The function to show the full screen in terminal
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

/* ------ example part ----- */

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

  while (1) {
    // cat idle demo
    cat_idle(repeat_count);

    // dog idle demo
    dog_idle(repeat_count);

    // selection demo
    select_character(repeat_count);

    // number icon demo
    num_test(repeat_once);
  }

  return 0;
}