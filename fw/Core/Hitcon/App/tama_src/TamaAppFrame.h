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
void stack_component(uint8_t* component, uint8_t* base,
                     component_info comp_info, base_info bs_info,
                     bool eliminate = false);

void stack_const_component(const uint8_t* component, uint8_t* base,
                           component_info comp_info, base_info bs_info);

void get_number_component(int target_num, uint8_t* base);

void get_warning_component(uint8_t* base);

void get_egg_component(int percentage, uint8_t* base);

void get_heart_overview_component(int heart_count, uint8_t* base);

void get_food_overview_component(int food_count, uint8_t* base);

void get_fd_icons_component(int food_count, uint8_t* base);

void get_hp_icons_component(int hp_count, uint8_t* base);

void get_hatch_status_frame(int remaining_count, uint8_t* base);

void get_hatch_born_warning_frame(int frame, uint8_t* base);

void get_dog_idle_frame_with_status_overview(int frame, int heart_count,
                                             int food_count, uint8_t* base);

void get_cat_idle_frame_with_status_overview(int frame, int heart_count,
                                             int food_count, uint8_t* base);

void get_pet_healing_frame(int pet_type, int frame, uint8_t* base);

void get_activity_selection_frame(int activity_type, int selection,
                                  uint8_t* base);

void get_select_character_frame(int frame, uint8_t* base);

void get_battle_result_frame(int pet, int result, int frame, uint8_t* base);

void get_battle_frame(int player_pet, int enemy_pet, int damage_target,
                      uint8_t* base);

void get_LV_status_frame(int level_number, uint8_t* base);

void get_FD_status_frame(int food_count, uint8_t* base);

void get_HP_status_frame(int hp_count, uint8_t* base);

void get_feed_confirm_frame(int selection, uint8_t* base);

void get_empty_frame(uint8_t* base);

void get_feed_pet_frame(int cookie_percent, uint8_t* base);

void get_pet_happy_frame_after_feed(int pet_type, int frame, uint8_t* base);

void get_scoring_frame(int ok_qty, int fail_qty, uint8_t* base);

void get_end_frame(uint8_t* base);

void get_feeding_frame(int per_type, int frame_ID, uint8_t* frame_buff);
#endif  // TAMA_APP_FRAME_H