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
void get_number_component(int target_num, uint8_t* base) {
  // check boundary of input
  if (target_num > 999) {
    target_num = 999;
  }
  if (target_num < 0) {
    target_num = 0;
  }

  // how many digits need to be displayed
  uint8_t digit_count = 1;
  if (target_num >= 10) {
    digit_count++;
  }
  if (target_num >= 100) {
    digit_count++;
  }

  // parse digits
  uint8_t digit_100x = target_num / 100;
  target_num = target_num % 100;
  uint8_t digit_10x = target_num / 10;
  uint8_t digit_1x = target_num % 10;

  constexpr base_info my_base_info = {
      .width = 8,
      .height = 8,
  };

  constexpr component_info digit_100x_component_info = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  constexpr component_info digit_10x_component_info = {
      .x_len = 2,
      .y_len = 8,
      .x_offset = 3,
      .y_offset = 0,
  };
  constexpr component_info digit_1x_component_info = {
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

  // stack number icon at 1x digit
  if (digit_count) {
    const CompressedImage* target = &m_num_icon_compressed[digit_1x];
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, digit_1x_component_info,
                    my_base_info);
    digit_count--;
  }

  // stack number icon at 10x digit
  if (digit_count) {
    const CompressedImage* target = &m_num_icon_compressed[digit_10x];
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, digit_10x_component_info,
                    my_base_info);
    digit_count--;
  }

  // stack number icon at 100x digit
  if (digit_count) {
    const CompressedImage* target = &m_num_icon_compressed[digit_100x];
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, digit_100x_component_info,
                    my_base_info);
    digit_count--;
  }
}

void get_fd_icons_component(int food_count, uint8_t* base) {
  constexpr base_info my_base_info = {
      .width = 8,
      .height = 8,
  };

  // the first food icon
  constexpr component_info food_component_info_LT = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 1,
      .y_offset = 1,
  };
  const CompressedImage* target = &m_food_icon_detail_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, food_component_info_LT,
                  my_base_info);

  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return;
  }

  // the second food icon
  constexpr component_info food_component_info_RT = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 5,
      .y_offset = 1,
  };
  const CompressedImage* target2 = &m_food_icon_detail_compressed;
  uint8_t decompressed_buffer2[target2->width * target2->height];
  memset(decompressed_buffer2, 0, target2->width * target2->height);
  decompress_component(target2, decompressed_buffer2);
  stack_component(decompressed_buffer2, base, food_component_info_RT,
                  my_base_info);
  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return;
  }

  // the third food icon
  constexpr component_info food_component_info_LB = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 0,
      .y_offset = 5,
  };
  const CompressedImage* target3 = &m_food_icon_detail_compressed;
  uint8_t decompressed_buffer3[target3->width * target3->height];
  memset(decompressed_buffer3, 0, target3->width * target3->height);
  decompress_component(target3, decompressed_buffer3);
  stack_component(decompressed_buffer3, base, food_component_info_LB,
                  my_base_info);
  food_count -= 1;
  if (food_count < 1) {
    // no more food, return the base with icon and one full square
    return;
  }

  // the fourth food icon
  constexpr component_info food_component_info_RB = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 4,
      .y_offset = 5,
  };
  const CompressedImage* target4 = &m_food_icon_detail_compressed;
  uint8_t decompressed_buffer4[target4->width * target4->height];
  memset(decompressed_buffer4, 0, target4->width * target4->height);
  decompress_component(target4, decompressed_buffer4);
  stack_component(decompressed_buffer4, base, food_component_info_RB,
                  my_base_info);
}

void get_hp_icons_component(int hp_count, uint8_t* base) {
  constexpr base_info my_base_info = {
      .width = 8,
      .height = 8,
  };

  // the first food icon
  constexpr component_info hp_component_info_top = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 3,
      .y_offset = 1,
  };

  const CompressedImage* target = &m_heart_icon_detail_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, hp_component_info_top,
                  my_base_info);
  hp_count -= 1;
  if (hp_count < 1) {
    // no more food, return the base with icon and one full square
    return;
  }

  // the second food icon
  constexpr component_info hp_component_info_bottom_left = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 0,
      .y_offset = 5,
  };

  const CompressedImage* target2 = &m_heart_icon_detail_compressed;
  uint8_t decompressed_buffer2[target2->width * target2->height];
  memset(decompressed_buffer2, 0, target2->width * target2->height);
  decompress_component(target2, decompressed_buffer2);
  stack_component(decompressed_buffer2, base, hp_component_info_bottom_left,
                  my_base_info);
  hp_count -= 1;
  if (hp_count < 1) {
    // no more food, return the base with icon and one full square
    return;
  }

  // the third food icon
  constexpr component_info hp_component_info_bottom_right = {
      .x_len = 3,
      .y_len = 3,
      .x_offset = 5,
      .y_offset = 5,
  };

  const CompressedImage* target3 = &m_heart_icon_detail_compressed;
  uint8_t decompressed_buffer3[target3->width * target3->height];
  memset(decompressed_buffer3, 0, target3->width * target3->height);
  decompress_component(target3, decompressed_buffer3);
  stack_component(decompressed_buffer3, base, hp_component_info_bottom_right,
                  my_base_info);
}

/** --- component part end---*/

void get_battle_result_frame(int pet, int result, int frame, uint8_t* base) {
  const component_info dog_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 5,
      .y_offset = 0,
  };
  const component_info cat_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 4,
      .y_offset = 0,
  };

  const component_info component_16x8 = {
      .x_len = 16,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  const base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  if (frame < 0 || frame > 1) {
    frame = 0;
  }

  if (pet == PET_TYPE_DOG) {
    const CompressedImage* target = &m_dog_battle_result_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, dog_component_info, screen_info);
  } else if (pet == PET_TYPE_CAT) {
    const CompressedImage* target = &m_cat_battle_result_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, cat_component_info, screen_info);
  }

  if (frame == FRAME_2) {
    // if frame is 2, no need to stack component
    return;
  }

  if (result == WIN) {
    const CompressedImage* target = &m_battle_result_win_effect_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, component_16x8, screen_info);
  } else if (result == LOSE) {
    const CompressedImage* target = &m_battle_result_lose_effect_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, component_16x8, screen_info);
  }
  // TODO: tie
}

void get_battle_frame(int player_pet, int enemy_pet, int damage_target,
                      uint8_t* base) {
  const component_info dog_player_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 1,
      .y_offset = 0,
  };
  const component_info cat_player_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };
  const component_info dog_enemy_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 9,
      .y_offset = 0,
  };
  const component_info cat_enemy_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 9,
      .y_offset = 0,
  };

  const component_info training_facility_enemy_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 9,
      .y_offset = 0,
  };

  const component_info damage_player_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  const component_info damage_enemy_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 8,
      .y_offset = 0,
  };

  const base_info screen_info = {
      .width = 16,
      .height = 8,
  };

  // stack player
  if (player_pet == PET_TYPE_DOG) {
    const CompressedImage* target = &m_player_dog_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, dog_player_component_info,
                    screen_info);
  } else if (player_pet == PET_TYPE_CAT) {
    const CompressedImage* target = &m_player_cat_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, cat_player_component_info,
                    screen_info);
  }

  // stack enemy
  if (enemy_pet == PET_TYPE_DOG) {
    const CompressedImage* target = &m_enemy_dog_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, dog_enemy_component_info,
                    screen_info);
  } else if (enemy_pet == PET_TYPE_CAT) {
    const CompressedImage* target = &m_enemy_cat_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, cat_enemy_component_info,
                    screen_info);
  } else if (enemy_pet == OTHER_TYPE_TRAINING_FACILITY) {
    const CompressedImage* target = &m_training_facility_enemy_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base,
                    training_facility_enemy_component_info, screen_info);
  }

  // stack damage effect
  if (damage_target == PLAYER) {
    const CompressedImage* target = &m_hit_player_effect_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, damage_player_component_info,
                    screen_info, true);
  } else if (damage_target == ENEMY) {
    const CompressedImage* target = &m_hit_enemy_effect_compressed;
    uint8_t decompressed_buffer[target->width * target->height];
    memset(decompressed_buffer, 0, target->width * target->height);
    decompress_component(target, decompressed_buffer);
    stack_component(decompressed_buffer, base, damage_enemy_component_info,
                    screen_info, true);
  }
}

void get_LV_status_frame(int level_number, uint8_t* base) {
  // check boundary of input
  if (level_number > 999) {
    level_number = 999;
  } else if (level_number < 0) {
    level_number = 0;
  }

  constexpr base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  constexpr component_info LV_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  constexpr component_info num_component_info = {
      .x_len = NUM_AREA_WIDTH,
      .y_len = NUM_AREA_HEIGHT,
      .x_offset = 8,
      .y_offset = 0,
  };

  /* combine hatch component with num component */
  // stack LV word icon
  const CompressedImage* target = &m_lv_word_icon_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, LV_component_info, my_base_info);

  // stack number icon
  if (level_number == 0) {
    // if level number is 0, return the base with LV word icon only
  }
  uint8_t number_component[num_component_info.x_len * num_component_info.y_len];
  memset(number_component, 0,
         num_component_info.x_len * num_component_info.y_len);
  get_number_component(level_number, number_component);
  stack_component(number_component, base, num_component_info, my_base_info);
}

void get_FD_status_frame(int food_count, uint8_t* base) {
  // check boundary of input
  if (food_count > 4) {
    food_count = 4;
  } else if (food_count < 0) {
    food_count = 0;
  }

  constexpr base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  constexpr component_info FD_word_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  constexpr component_info FD_icons_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 8,
      .y_offset = 0,
  };

  /* combine hatch component with num component */
  // stack LV word icon
  const CompressedImage* target = &m_fd_word_icon_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, FD_word_component_info,
                  my_base_info);

  if (food_count == 0) {
    // if food count is 0, return the base with FD word icon only
    return;
  }
  // stack number icon
  uint8_t food_count_component[FD_icons_component_info.x_len *
                               FD_icons_component_info.y_len];
  memset(food_count_component, 0,
         FD_icons_component_info.x_len * FD_icons_component_info.y_len);
  get_fd_icons_component(food_count, food_count_component);
  stack_component(food_count_component, base, FD_icons_component_info,
                  my_base_info);
}

void get_HP_status_frame(int hp_count, uint8_t* base) {
  // check boundary of input
  if (hp_count > 3) {
    hp_count = 3;
  } else if (hp_count < 0) {
    hp_count = 0;
  }

  constexpr base_info my_base_info = {
      .width = DISPLAY_WIDTH,
      .height = DISPLAY_HEIGHT,
  };

  constexpr component_info HP_word_component_info = {
      .x_len = 7,
      .y_len = 8,
      .x_offset = 0,
      .y_offset = 0,
  };

  constexpr component_info HP_icons_component_info = {
      .x_len = 8,
      .y_len = 8,
      .x_offset = 8,
      .y_offset = 0,
  };

  /* combine hatch component with num component */
  // stack LV word icon
  const CompressedImage* target = &m_hp_word_icon_compressed;
  uint8_t decompressed_buffer[target->width * target->height];
  memset(decompressed_buffer, 0, target->width * target->height);
  decompress_component(target, decompressed_buffer);
  stack_component(decompressed_buffer, base, HP_word_component_info,
                  my_base_info);
  if (hp_count == 0) {
    // if hp count is 0, return the base with HP word icon only
    return;
  }
  // stack number icon
  uint8_t hp_icons_component[HP_icons_component_info.x_len *
                             HP_icons_component_info.y_len];
  memset(hp_icons_component, 0,
         HP_icons_component_info.x_len * HP_icons_component_info.y_len);
  get_hp_icons_component(hp_count, hp_icons_component);
  stack_component(hp_icons_component, base, HP_icons_component_info,
                  my_base_info);
}

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
