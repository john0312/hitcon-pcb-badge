#define constexpr const
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 8
#define SLEEP_US 500000
#define CAT_IDLE_FRAME_COUNT 2
#define DOG_IDLE_FRAME_COUNT 2
#define SELECT_CHARACTER_FRAME_COUNT 2
#define NEW_SCREEN NULL
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

using hitcon::app::tama::screens::m_cat_idle1;
using hitcon::app::tama::screens::m_cat_idle2;
using hitcon::app::tama::screens::m_dog_idle1;
using hitcon::app::tama::screens::m_dog_idle2;
using hitcon::app::tama::screens::m_select_cursor;
using hitcon::app::tama::screens::m_select_print_all_character;

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
                             component_info comp_info) {
  // use a new base if input is empty
  if (base == NULL) {
    base = new uint8_t[DISPLAY_WIDTH *
                       DISPLAY_HEIGHT]();  // create a new memory at heap
  }

  // stack new component onto base
  for (int y = 0; y < comp_info.y_len; ++y) {
    for (int x = 0; x < comp_info.x_len; ++x) {
      base[(comp_info.y_offset + y) * DISPLAY_WIDTH +
           (comp_info.x_offset + x)] =
          component[y * comp_info.x_len + x] |
          base[(comp_info.y_offset + y) * DISPLAY_WIDTH +
               (comp_info.x_offset + x)];
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

  uint8_t* cat_idle1 =
      stack_target_offset(m_cat_idle1, NEW_SCREEN, cat_idle_component_info);
  uint8_t* cat_idle2 =
      stack_target_offset(m_cat_idle2, NEW_SCREEN, cat_idle_component_info);
  const uint8_t* cat_idle_frame_all[CAT_IDLE_FRAME_COUNT] = {cat_idle1,
                                                             cat_idle2};
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(cat_idle_frame_all, CAT_IDLE_FRAME_COUNT, SLEEP_US);
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
  uint8_t* dog_idle1 =
      stack_target_offset(m_dog_idle1, NEW_SCREEN, dog_idle_component_info);
  uint8_t* dog_idle2 =
      stack_target_offset(m_dog_idle2, NEW_SCREEN, dog_idle_component_info);
  const uint8_t* dog_idle_frame_all[DOG_IDLE_FRAME_COUNT] = {dog_idle1,
                                                             dog_idle2};
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(dog_idle_frame_all, DOG_IDLE_FRAME_COUNT, SLEEP_US);
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
  uint8_t* select_print_all_character1 =
      stack_target_offset(m_select_print_all_character, NEW_SCREEN,
                          select_print_all_character_component_info);
  uint8_t* select_left = stack_target_offset(
      m_select_cursor, select_print_all_character1, select_left_component_info);

  uint8_t* select_print_all_character2 =
      stack_target_offset(m_select_print_all_character, NEW_SCREEN,
                          select_print_all_character_component_info);
  uint8_t* select_right =
      stack_target_offset(m_select_cursor, select_print_all_character2,
                          select_right_component_info);
  const uint8_t* select_character_frame_all[SELECT_CHARACTER_FRAME_COUNT] = {
      select_left, select_right};
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(select_character_frame_all,
                          SELECT_CHARACTER_FRAME_COUNT, SLEEP_US);
  }
}

int main() {
  int repeat_count = 3;

  while (1) {
    // cat idle demo
    cat_idle(repeat_count);

    // dog idle demo
    dog_idle(repeat_count);

    // selection demo
    select_character(repeat_count);
  }

  return 0;
}