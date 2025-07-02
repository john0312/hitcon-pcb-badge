// #define SIMU  // open if needed

#ifdef SIMU
#define constexpr const
#define SLEEP_US 500000

#include "TamaAppFrame.h"

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

  uint8_t* cat_idle1 =
      stack_component(decompress_component(&m_cat_idle1_compressed), NEW_SCREEN,
                      cat_idle_component_info, screen_info);
  uint8_t* cat_idle2 =
      stack_component(decompress_component(&m_cat_idle2_compressed), NEW_SCREEN,
                      cat_idle_component_info, screen_info);
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
  uint8_t* dog_idle1 =
      stack_component(decompress_component(&m_dog_idle1_compressed), NEW_SCREEN,
                      dog_idle_component_info, screen_info);
  uint8_t* dog_idle2 =
      stack_component(decompress_component(&m_dog_idle2_compressed), NEW_SCREEN,
                      dog_idle_component_info, screen_info);
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

  uint8_t* select_print_all_character1 = stack_component(
      decompress_component(&m_select_print_all_character_compressed),
      NEW_SCREEN, select_print_all_character_component_info, screen_info);
  uint8_t* select_left = stack_component(
      decompress_component(&m_select_cursor_compressed),
      select_print_all_character1, select_left_component_info, screen_info);

  uint8_t* select_print_all_character2 = stack_component(
      decompress_component(&m_select_print_all_character_compressed),
      NEW_SCREEN, select_print_all_character_component_info, screen_info);
  uint8_t* select_right = stack_component(
      decompress_component(&m_select_cursor_compressed),
      select_print_all_character2, select_right_component_info, screen_info);
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
      .y_len = 8,
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
    frame_all[i] =
        stack_target_offset(num_component_all[i], NEW_SCREEN,
                            num_area_component_info, screen_info, true);
  }

  // show animation
  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(frame_all, frame_count, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < frame_count; ++i) {
    delete[] frame_all[i];
  }
}

/**
 * @brief The example of cat_idle
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void cat_idle_with_status(int repeat_count) {
  const uint8_t* cat_idle1 = get_cat_idle_frame_with_status_overview(
      FRAME_1, 3, 4);  // dog idle frame with status overview
  const uint8_t* cat_idle2 = get_cat_idle_frame_with_status_overview(
      FRAME_2, 3, 4);  // dog idle frame with status overview

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
void dog_idle_with_status(int repeat_count) {
  const uint8_t* dog_idle1 = get_dog_idle_frame_with_status_overview(
      FRAME_1, 3, 4);  // dog idle frame with status overview
  const uint8_t* dog_idle2 = get_dog_idle_frame_with_status_overview(
      FRAME_2, 3, 4);  // dog idle frame with status overview

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
 * @brief The example of cat_idle with weak status
 *
 * Same as cat_idle_with_status, only passing value different.
 * The case here is hungry trigger weak status.
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void cat_weak_idle_with_status(int repeat_count) {
  const uint8_t* cat_idle1 = get_cat_idle_frame_with_status_overview(
      FRAME_1, 3, 0);  // dog idle frame with status overview
  const uint8_t* cat_idle2 = get_cat_idle_frame_with_status_overview(
      FRAME_2, 3, 0);  // dog idle frame with status overview

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
 * @brief The example of dog_idle with weak status
 *
 * Same as dog_idle_with_status, only passing value different.
 * The case here is heart poor trigger weak status.
 *
 * @param repeat_count How many times should frame_collection repeat
 */

void dog_weak_idle_with_status(int repeat_count) {
  const uint8_t* dog_idle1 = get_dog_idle_frame_with_status_overview(
      FRAME_1, 0, 4);  // dog idle frame with status overview
  const uint8_t* dog_idle2 = get_dog_idle_frame_with_status_overview(
      FRAME_2, 0, 4);  // dog idle frame with status overview

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

void dog_status_change(int repeat_count) {
  const uint8_t* dog_status1 = get_dog_idle_frame_with_status_overview(
      FRAME_1, 3, 4);  // dog idle frame with status overview
  const uint8_t* dog_status2 = get_dog_idle_frame_with_status_overview(
      FRAME_2, 2, 2);  // dog idle frame with status overview
  const uint8_t* dog_status3 = get_dog_idle_frame_with_status_overview(
      FRAME_1, 1, 0);  // dog idle frame with status overview

  const uint8_t* dog_idle_frame_all[3] = {dog_status1, dog_status2,
                                          dog_status3};

  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(dog_idle_frame_all, 3, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < 3; ++i) {
    delete[] dog_idle_frame_all[i];
  }
}

/**
 * @brief The example of pet healing
 *
 * This function will show the pet healing animation.
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void pet_healing(int pet_type, int repeat_count) {
  const uint8_t frame_count = 2;
  const uint8_t* pet_healing1 = get_pet_healing_frame(pet_type, FRAME_1);
  const uint8_t* pet_healing2 = get_pet_healing_frame(pet_type, FRAME_2);

  // heart float up > empty
  const uint8_t* pet_healing_frame_all[frame_count] = {
      pet_healing1,
      pet_healing2,
  };

  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(pet_healing_frame_all, frame_count, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < frame_count; ++i) {
    delete[] pet_healing_frame_all[i];
  }
}

/**
 * @brief The example of training_selection
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void training_selection(int repeat_count) {
  const uint8_t frame_count = 2;
  const uint8_t* frame_left = get_activity_selection_frame(TRAINING, LEFT);
  const uint8_t* frame_right = get_activity_selection_frame(TRAINING, RIGHT);

  const uint8_t* frame_all[frame_count] = {
      frame_left,
      frame_right,
  };

  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(frame_all, frame_count, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < SELECT_YN_FRAME_COUNT; ++i) {
    delete[] frame_all[i];
  }
}

/**
 * @brief The example of battle_selection
 *
 * @param repeat_count How many times should frame_collection repeat
 */
void battle_selection(int repeat_count) {
  const uint8_t frame_count = 2;
  const uint8_t* frame_left = get_activity_selection_frame(BATTLE, LEFT);
  const uint8_t* frame_right = get_activity_selection_frame(BATTLE, RIGHT);

  const uint8_t* frame_all[frame_count] = {
      frame_left,
      frame_right,
  };

  for (int i = 0; i < repeat_count; ++i) {
    show_anime_with_delay(frame_all, frame_count, SLEEP_US);
  }

  // loop to release all allocated memory
  for (int i = 0; i < SELECT_YN_FRAME_COUNT; ++i) {
    delete[] frame_all[i];
  }
}

void test_frames() {
  int repeat_count = 3;
  int repeat_once = 1;

  // cat idle demo
  // std::cout << "Cat Idle Demo:\n";
  // cat_idle(repeat_count);

  // dog idle demo
  // std::cout << "Dog Idle Demo:\n";
  // dog_idle(repeat_count);

  // selection demo
  std::cout << "Select Character Demo:\n";
  select_character(repeat_count);

  // number icon demo
  // std::cout << "Number Icon Demo:\n";
  // num_test(repeat_once);

  // egg hatch demo
  std::cout << "Egg Hatch Demo:\n";
  egg_hatch(repeat_once);

  // cat idle demo with status
  std::cout << "Cat Idle Demo with status:\n";
  cat_idle_with_status(repeat_count);

  // dog idle demo with status
  std::cout << "Dog Idle Demo with status:\n";
  dog_idle_with_status(repeat_count);

  // dog idle demo with status
  // std::cout << "Dog status change Demo:\n";
  // dog_status_change(repeat_count);

  // cat idle demo with status
  std::cout << "Cat Weak Idle Demo with status:\n";
  cat_weak_idle_with_status(repeat_count);

  // dog idle demo with status
  std::cout << "Dog Weak Idle Demo with status:\n";
  dog_weak_idle_with_status(repeat_count);

  // dog healing demo
  std::cout << "Dog healing Demo:\n";
  pet_healing(PET_TYPE_DOG, repeat_count);

  // cat healing demo
  std::cout << "Cat healing Demo:\n";
  pet_healing(PET_TYPE_CAT, repeat_count);

  // training selection demo
  std::cout << "training_selection Demo:\n";
  training_selection(repeat_count);

  // battle selection demo
  std::cout << "battle_selection Demo:\n";
  battle_selection(repeat_count);
}

void test_compress_decompress() {
  //---- compress part, can use for converting image to compressed data ---
  compress_data_and_print_info(m_example_of_compress, 8, 8);

  //---- decompress part, can use for verifying compressed data ----
  // constexpr uint8_t hardcoded_compressed_data[] = {0x38, 0xE0, 0x70, 0xF8,
  //                                                  0x7C, 0xF8, 0x7C, 0x10};

  // CompressedImage hardcoded_compressed_image = {
  //     .width = 8, .height = 8, .data = hardcoded_compressed_data};

  // decompress_and_print_component(&hardcoded_compressed_image);
}

int main() {
  // test_frames();

  test_compress_decompress();  // open it if needed
  return 0;
}

#endif
