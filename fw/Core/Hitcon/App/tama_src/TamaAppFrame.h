#ifndef TAMA_APP_FRAME_H
#define TAMA_APP_FRAME_H

#include "TamaAppUtils.h"

#define NEW_SCREEN NULL

using namespace hitcon::app::tama::components;
using namespace hitcon::app::tama::egg_icon;
using namespace hitcon::app::tama::menu_icon;

// Constants
constexpr int CAT_IDLE_FRAME_COUNT = 2;
constexpr int DOG_IDLE_FRAME_COUNT = 2;
constexpr int SELECT_CHARACTER_FRAME_COUNT = 2;
constexpr int SELECT_YN_FRAME_COUNT = 2;
constexpr int HATCH_WARNING_REPEAT_SHINE_COUNT = 3;
constexpr int HATCH_STATUS_FRAME_COUNT = 4;
constexpr int HATCH_START_COUNT = 400;
constexpr int NUM_AREA_WIDTH = 8;
constexpr int NUM_AREA_HEIGHT = 8;
constexpr int EGG_AREA_WIDTH = 8;
constexpr int EGG_AREA_HEIGHT = 8;
constexpr int FOOD_HEART_OVERVIEW_COMPONENT_WIDTH = 8;
constexpr int FOOD_HEART_OVERVIEW_COMPONENT_HEIGHT = 4;

// Struct definitions
struct component_info {
  uint8_t x_len;
  uint8_t y_len;
  uint8_t x_offset;
  uint8_t y_offset;
};

struct base_info {
  uint8_t width;
  uint8_t height;
};

// Function declarations
uint8_t* stack_component(uint8_t* component, uint8_t* base,
                         component_info comp_info, base_info bs_info,
                         bool eliminate = false);

uint8_t* stack_const_component(const uint8_t* component, uint8_t* base,
                               component_info comp_info, base_info bs_info);

uint8_t* get_number_component(int target_num);

uint8_t* get_warning_component();

uint8_t* get_egg_component(int percentage);

uint8_t* get_heart_overview_component(int heart_count);

uint8_t* get_food_overview_component(int food_count);

uint8_t* get_fd_icons_component(int food_count);

uint8_t* get_hp_icons_component(int hp_count);

const uint8_t* get_hatch_status_frame(int remaining_count);

const uint8_t* get_hatch_born_warning_frame(int frame);

const uint8_t* get_dog_idle_frame_with_status_overview(int frame,
                                                       int heart_count,
                                                       int food_count);

const uint8_t* get_cat_idle_frame_with_status_overview(int frame,
                                                       int heart_count,
                                                       int food_count);

const uint8_t* get_pet_healing_frame(int pet_type, int frame);

const uint8_t* get_activity_selection_frame(int activity_type, int selection);

const uint8_t* get_select_character_frame(int frame);

const uint8_t* get_battle_result_frame(int pet, int result, int frame);

const uint8_t* get_battle_frame(int player_pet, int enemy_pet,
                                int damage_target);

const uint8_t* get_LV_status_frame(int level_number);

const uint8_t* get_FD_status_frame(int food_count);

const uint8_t* get_HP_status_frame(int hp_count);

const uint8_t* get_feed_confirm_frame(int selection);

uint8_t* get_empty_frame();

const uint8_t* get_feed_pet_frame(int cookie_percent);

const uint8_t* get_pet_happy_frame_after_feed(int pet_type, int frame);

const uint8_t* get_scoring_frame(int ok_qty, int fail_qty);

const uint8_t* get_end_frame();

#endif  // TAMA_APP_FRAME_H