#ifndef TAMA_APP_H
#define TAMA_APP_H
#define TAMA_APP_MAX_FB_LENGTH 6
#define TAMA_HATCHING_STEPS 400
#define TAMA_HUNGER_DECREASE_INTERVAL 3600000

#define TAMA_PREPARE_FB(FB, FB_SIZE) \
  FB.fb_size = FB_SIZE;              \
  memset(FB.fb, 0, sizeof(FB.fb[0]) * FB_SIZE);
#define TAMA_GET_ANIMATION_DATA(TYPE_NAME_STR) \
  animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)]
#define TAMA_COPY_FB(FB, ANIMATION, OFFSET)                      \
  FB.active_frame = 0;                                           \
  FB.fb_size = (ANIMATION).frame_count;                          \
  for (int m = 0; m < (ANIMATION).frame_count; m++) {            \
    for (int n = 0; n < (ANIMATION).length; n++) {               \
      my_assert(n + OFFSET < DISPLAY_WIDTH);                     \
      FB.fb[m][n + OFFSET] =                                     \
          (ANIMATION).frames_data[((ANIMATION).length) * m + n]; \
    }                                                            \
  }

#include <Logic/Display/display.h>
#include <Logic/ImuLogic.h>
#include <Logic/IrController.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/SysTimer.h>  // For SysTimer

#include <cstdarg>

#include "app.h"

namespace hitcon {
namespace app {
namespace tama {

enum class TAMA_APP_STATE : uint8_t {
  INTRO_TEXT,   // Displaying introductory text
  CHOOSE_TYPE,  // Player is selecting a pet type
  EGG_1,        // 0% hatching progress
  EGG_2,        // 25% hatching progress
  EGG_3,        // 50% hatching progress
  EGG_4,        // 75% hatching progress
  HATCHING,     // 100% animation
  IDLE,
  LV_DETAIL,
  FEED_CONFIRM,
  FEED_ANIME,
  PET_FED,
  PET_HEALING,
  TRAINING_CONFIRM,
  TRAINING,
  TRAINING_QTE,
  TRAINING_END
};

enum class TAMA_TYPE : uint8_t {
  NONE_TYPE,  // No pet selected or game reset
  DOG,        // Player chose a dog
  CAT,        // Player chose a cat
};

typedef struct {
  TAMA_APP_STATE state;
  TAMA_TYPE type;
  uint16_t level;
  uint8_t hp;
  uint8_t hunger;
} tama_storage_t;

typedef struct {
  display_buf_t fb[TAMA_APP_MAX_FB_LENGTH][DISPLAY_WIDTH];
  uint8_t fb_size;
  uint8_t active_frame;
} tama_display_fb_t;

enum class TAMA_PLAYER_MODE : uint8_t {
  MODE_SINGLEPLAYER,
  MODE_MULTIPLAYER,
  MODE_BASESTATION,
};

enum class TAMA_XBOARD_PACKET_TYPE {
  // TODO: Add all packet type
  PACKET_CONFIRM,
  PACKET_ENIMY_INFO,
  PACKET_SCORE,
  PACKET_END,
  PACKET_LEAVE,
  PACKET_UNAVAILABLE,
};

enum class TAMA_XBOARD_STATE {
  XBOARD_INVITE,
  XBOARD_BATTLE_ENCOUNTER,
  XBOARD_BATTLE_QTE,
  XBOARD_BATTLE_SENT_SCORE,
  XBOARD_BATTLE_RESULT,
  XBOARD_BATTLE_END,
  XBOARD_UNAVAILABLE,
};

enum class TAMA_XBOARD_BATTLE_INVITE {
  XBOARD_BATTLE_N,
  XBOARD_BATTLE_Y,
};

enum class TAMA_ANIMATION_TYPE : uint8_t {
  DOG_IDLE,
  CAT_IDLE,
  DOG_WEAK,
  CAT_WEAK,
  EGG_1,
  EGG_2,
  EGG_3,
  EGG_4,
  HATCHING,
  PET_SELECTION,
  FEED_CONFIRM,
  FEEDING,
  TRAINING_CONFIRM,
  DOG_FED_HEALING,
  CAT_FED_HEALING,
  HEART_3,
  HEART_2,
  HEART_1,
  NEED_HEAL,
  LV,
  XB_BATTLE_INVITE,
  XB_PLAYER_DOG,
  XB_PLAYER_CAT,
  XB_ENEMY_DOG,
  XB_ENEMY_CAT,
  XB_PLAYER_DOG_HURT,
  XB_PLAYER_CAT_HURT,
  XB_ENEMY_DOG_HURT,
  XB_ENEMY_CAT_HURT,
  XB_DOG_RESULT,
  XB_CAT_RESULT,
};

typedef struct {
  TAMA_XBOARD_PACKET_TYPE packet_type;
  uint8_t user[hitcon::ir::IR_USERNAME_LEN];
  uint16_t score;
  uint8_t nonce;
} __attribute__((__packed__)) tama_xboard_result_t;

typedef struct {
  TAMA_XBOARD_PACKET_TYPE packet_type;
  TAMA_TYPE type;
} tama_xboard_enemy_info_t;

constexpr uint8_t QTE_REFRESH_RATE = 50;
constexpr uint16_t PAUSE_BETWEEN_QTE_GAMES = 1000;
constexpr uint8_t QTE_TOTAL_ROUNDS = 5;
constexpr uint16_t CRITICAL_HIT_SCORE = UINT16_MAX;

class TamaQteArrow {
 private:
  int8_t location;
  int8_t direction;
  bool done;

 public:
  void Init();
  void Render(display_buf_t* buf);
  void Update();
  bool IsDone();
  uint8_t GetLocation();
};

class TamaQteTarget {
 private:
  uint8_t location;

 public:
  void Init();
  void Render(display_buf_t* buf);
  uint8_t GetLocation();
};

class TamaQteGame {
 private:
  TamaQteArrow arrow;
  TamaQteTarget target;
  bool done;
  bool success;

 public:
  void Init();
  bool IsDone();
  void Render(display_buf_t* buf);
  void Update();
  bool IsSuccess();
  void OnButton();
};

enum TamaQteState { kDone = 0, kInGame, kBetweenGame };

class TamaQte {
 private:
  TamaQteState state;
  TamaQteGame game;
  unsigned int nextGameStart;
  hitcon::service::sched::PeriodicTask routineTask;
  uint8_t currentRound;
  uint8_t success;
  uint16_t score;
  void Routine();
  void Render();
  void SaveScore();

 public:
  TamaQte();
  // Init the qte. This should be called only once on startup.
  void Init();
  // Start the qte game. This should be called every time QTE starts.
  void Entry();
  void Exit();
  void OnButton(button_t button);
  bool IsDone();
  uint16_t GetScore(uint16_t level);
  uint8_t GetSuccess();
};

typedef struct {
  TAMA_ANIMATION_TYPE type;
  uint8_t frame_count;
  uint8_t length;
  const display_buf_t* frames_data;
} tama_ani_t;

typedef struct {
  const display_buf_t* data;
  uint8_t length;
} tama_display_component_t;

class TamaApp : public App {
 private:
  static constexpr unsigned ROUTINE_INTERVAL_MS =
      500;  // How often the Routine function runs
  TAMA_APP_STATE _state;
  TAMA_TYPE _current_selection_in_choose_mode;
  hitcon::service::sched::PeriodicTask _routine_task;
  hitcon::service::sched::DelayedTask _hatching_task;
  hitcon::service::sched::PeriodicTask _hunger_task;
  tama_storage_t& _tama_data;
  tama_display_fb_t _fb;
  unsigned int _frame_count = 0;
  bool _is_selected = false;
  unsigned int _previous_hatching_step = 0;
  unsigned int _total_hatchin_steps = 0;
  unsigned int _hunger_check_elapsed = 0;
  unsigned int _last_hunger_check = 0;
  bool _xb_qte_me_winning = false;
  bool _xb_qte_enemy_winning = false;

  TamaQte qte;

  void SetHunger(uint8_t hunger);
  void Render();
  void Routine(void* unused);
  void UpdateFrameBuffer();
  void StackOnFrame(const tama_display_component_t* component, int offset);
  void StackOnFrameBlinking(const tama_display_component_t* component,
                            int offset);
  void StackOnFrameShifing(const tama_display_component_t* component,
                           int offset);
  void ConcateAnimtaions(uint8_t count, ...);
  void HatchingRoutine(void* unused);
  void HungerRoutine(void* unused);

  // XBoard related
  TAMA_XBOARD_STATE _enemy_state;
  uint8_t _my_nounce;
  tama_xboard_enemy_info_t _enemy_info;
  tama_xboard_result_t _enemy_score;
  void XbOnButton(button_t button);
  void XbUpdateFrameBuffer();
  void XbRoutine(void* unused);

 public:
  TAMA_PLAYER_MODE player_mode;
  TamaApp();
  virtual ~TamaApp() = default;
  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  // void OnEdgeButton(button_t button) override;

  // XBoard related
  TAMA_XBOARD_STATE xboard_state;
  TAMA_XBOARD_BATTLE_INVITE xboard_battle_invite;
  void OnXBoardRecv(void* arg);

  // BaseStation
  void TamaHeal();
};

void SetSingleplayer();
void SetMultiplayer();
void SetBaseStationConnect();

extern TamaApp tama_app;

// --- Animation Frame Data Definitions (for tama_ani_t and static asserts) ---
// Each frame is 8 columns wide. Characters are 5 columns wide and centered.
// clang-format off
constexpr display_buf_t TAMA_DOG_IDLE_FRAMES[] = {
  // size 2x8
  0x38, 0xE0, 0x70, 0xF8, 0x7C, 0xF8, 0x7C, 0x10,
  0x30, 0xE0, 0x70, 0xF8, 0x7C, 0xF8, 0x7C, 0x10,
};
constexpr display_buf_t TAMA_CAT_IDLE_FRAMES[] = {
  // size 2x8
  0x00, 0x30, 0xC8, 0x60, 0xF8, 0x70, 0xF8, 0x00,
  0x08, 0x28, 0xD0, 0x60, 0xF8, 0x70, 0xF8, 0x00,
};
constexpr display_buf_t TAMA_DOG_WEAK_FRAMES[] = {
  // size 2x8
  0x20, 0xE0, 0xC8, 0xF0, 0xE2, 0xF0, 0x44, 0x00,
  0x20, 0xE0, 0xC4, 0xF0, 0xE0, 0xF0, 0x42, 0x00,
};
constexpr display_buf_t TAMA_CAT_WEAK_FRAMES[] = {
  // size 2x8
  0x10, 0x50, 0xA8, 0xC0, 0xF2, 0xE0, 0xF4, 0x00,
  0x10, 0x50, 0xA4, 0xC0, 0xF4, 0xE0, 0xF2, 0x00,
};
constexpr display_buf_t TAMA_EGG_1_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0xF8, 0xFC, 0xFC, 0xF8, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0x00,
  0x00, 0x38, 0x7c, 0x7e, 0x7e, 0x7c, 0x38, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_2_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0xB8, 0xFC, 0xF4, 0x78, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x38, 0x5c, 0x7e, 0x7a, 0x3c, 0x38, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_3_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0xA8, 0xFC, 0xE4, 0x58, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x38, 0x54, 0x7e, 0x72, 0x2c, 0x38, 0x00, 0x00, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_4_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0x28, 0x0C, 0xA0, 0x98, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x38, 0x14, 0x06, 0x50, 0x4c, 0x38, 0x00, 0x00, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_HATCHING_FRAMES[] = {
  // size 2x16
  0x10, 0x54, 0x00, 0xC6, 0x00, 0x54, 0x10, 0x00, 0x00, 0x00, 0b01011110, 0x00, 0b01011110, 0x00, 0b01011110, 0x00,
  0x00, 0x10, 0x28, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0b01011110, 0x00, 0b01011110, 0x00, 0b01011110, 0x00,
};
constexpr display_buf_t TAMA_PET_SELECTION_FRAMES[] = {
  // size 1x16
  0x00, 0x18, 0x60, 0x30, 0x7C, 0x38, 0x7C, 0x00, 0x18, 0x70, 0x38, 0x7E, 0x3C, 0x7E, 0x3C, 0x08
};
constexpr display_buf_t TAMA_FEED_CONFIRM_FRAMES[] = {
  // size 1x8
  0b00111000, 0b001000100, 0b01110010, 0b01110001, 0b01110001, 0b01110010, 0b01000100, 0b00111000
};
constexpr display_buf_t TAMA_FEEDING_FRAMES[] = {
  // size 5x8
  0b00111000, 0b01000100, 0b01110010, 0b01110001, 0b01110001, 0b01110010, 0b01000100, 0b00111000,
  0b00111000, 0b01000100, 0b01110010, 0b01110001, 0b01110111, 0b01111000, 0b01001000, 0b00111000,
  0b00110000, 0b01010000, 0b01110000, 0b01110000, 0b01111000, 0b01111000, 0b01001000, 0b00111000,
  0, 0b01100000, 0b01110000, 0b01100000, 0, 0b010000, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};
constexpr display_buf_t TAMA_TRAINING_CONFIRM_FRAMES[] = {
  // size 1x8
  0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c
};
constexpr display_buf_t TAMA_DOG_FED_HEALING_FRAMES[] = {
  // size 2x8
  0x20, 0xE0, 0xC0, 0xF0, 0xE0, 0xF0, 0x40, 0x00,
  0x20, 0xE0, 0xC0, 0xF2, 0xE4, 0xF2, 0x40, 0x00,
};
constexpr display_buf_t TAMA_CAT_FED_HEALING_FRAMES[] = {
  // size 2x8
  0x10, 0x50, 0xA0, 0xC0, 0xF0, 0xE0, 0xF0, 0x00,
  0x10, 0x50, 0xA0, 0xC2, 0xF4, 0xE2, 0xF0, 0x00,
};
constexpr display_buf_t TAMA_HEART_3_FRAMES[] = {
  // size 2x8
  0x0, 0b01100000, 0b11000000, 0b01100011, 0b00000110, 0b01100011, 0b11000000, 0b01100000,
  0x0, 0b00110000, 0b01100000, 0b00110110, 0b00001100, 0b00110110, 0b01100000, 0b00110000,
};
constexpr display_buf_t TAMA_HEART_2_FRAMES[] = {
  // size 2x8
  0x0, 0b11, 0b110, 0b011, 0x0, 0b01100000, 0b11000000, 0b01100000,
  0x0, 0b110, 0b1100, 0b110, 0x0, 0b00110000, 0b01100000, 0b00110000,
};
constexpr display_buf_t TAMA_HEART_1_FRAMES[] = {
  // size 2x8
  0x0, 0x0, 0x0, 0b1100, 0b11000, 0b1100, 0x0, 0x0,
  0x0, 0x0, 0x0, 0b11000, 0b110000, 0b11000, 0x0, 0x0,
};
constexpr display_buf_t TAMA_NEED_HEAL_FRAMES[] = {
  // size 2x8
  0, 0x60, 0xf0, 0xf8, 0x74, 0x22, 0x12, 0x0C,
  0, 0x30, 0x78, 0x7c, 0x3a, 0x11, 0x09, 0x06,
};
constexpr display_buf_t TAMA_LV_FRAMES[] = {
  // size 2x4
  0b00001110, 0b01101000, 0b10000000, 0b01100000,
  0b00000111, 0b00110100, 0b01000000, 0b00110000,
};
constexpr display_buf_t TAMA_XB_BATTLE_INVITE_FRAMES[] = {
  // size 1x8
  0x80, 0x58, 0x30, 0x68, 0x54, 0x0A, 0x05, 0x03
};
constexpr display_buf_t TAMA_XB_PLAYER_DOG_FRAMES[] = {
  // size 2x8
  0x20, 0xE0, 0xC0, 0xF0, 0xE0, 0xF0, 0x40, 0,
  0, 0x20, 0xE0, 0xC0, 0xF0, 0xE0, 0xF0, 0x40,
};
constexpr display_buf_t TAMA_XB_PLAYER_CAT_FRAMES[] = {
  // size 2x8
  0x10, 0x50, 0xA0, 0xC0, 0xF0, 0xE0, 0xF0, 0,
  0, 0x10, 0x50, 0xA0, 0xC0, 0xF0, 0xE0, 0xF0,
};
constexpr display_buf_t TAMA_XB_ENEMY_DOG_FRAMES[] = {
  // size 2x8
  0, 0x04, 0x1F, 0x0E, 0x1F, 0x0C, 0x1C, 0x06,
  0x04, 0x1F, 0x0E, 0x1F, 0x0C, 0x1C, 0x06, 0,
};
constexpr display_buf_t TAMA_XB_ENEMY_CAT_FRAMES[] = {
  // size 2x8
  0, 0x1f, 0x0e, 0x1f, 0x0c, 0x1a, 0x05, 0x01,
  0x1f, 0x0e, 0x1f, 0x0c, 0x1a, 0x05, 0x01, 0,
};
constexpr display_buf_t TAMA_XB_PLAYER_DOG_HURT_FRAMES[] = {
  // size 2x8
  0x20, 0xE0, 0xC0, 0xF0, 0xE0, 0xF0, 0x40, 0,
  0x28, 0xF0, 0xC0, 0xF8, 0xF0, 0xF0, 0x40, 0x80,
};
constexpr display_buf_t TAMA_XB_PLAYER_CAT_HURT_FRAMES[] = {
  // size 2x8
  0x10, 0x50, 0xA0, 0xC0, 0xF0, 0xE0, 0xF0, 0,
  0x18, 0x50, 0xa0, 0xc8, 0xf0, 0xe0, 0xf0, 0x80,
};
constexpr display_buf_t TAMA_XB_ENEMY_DOG_HURT_FRAMES[] = {
  0, 0x04, 0x1F, 0x0E, 0x1F, 0x0C, 0x1C, 0x06,
  0x01, 0x06, 0x1f, 0x1f, 0x1f, 0x0c, 0x1c, 0x16
  // size 2x8
};
constexpr display_buf_t TAMA_XB_ENEMY_CAT_HURT_FRAMES[] = {
  // size 2x8
  0, 0x1f, 0x0e, 0x1f, 0x0c, 0x1a, 0x05, 0x01,
  0x01, 0x1f, 0x0e, 0x1f, 0x1e, 0x1e, 0x0d, 0x11
};
// clang-format on

// --- Animation Definition Structure --

constexpr tama_ani_t animation[] = {
    {.type = TAMA_ANIMATION_TYPE::DOG_IDLE,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_DOG_IDLE_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT_IDLE,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_CAT_IDLE_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::DOG_WEAK,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_DOG_WEAK_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT_WEAK,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_CAT_WEAK_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::EGG_1,
     .frame_count = 2,
     .length = 16,
     .frames_data = TAMA_EGG_1_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::EGG_2,
     .frame_count = 2,
     .length = 16,
     .frames_data = TAMA_EGG_2_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::EGG_3,
     .frame_count = 2,
     .length = 16,
     .frames_data = TAMA_EGG_3_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::EGG_4,
     .frame_count = 2,
     .length = 16,
     .frames_data = TAMA_EGG_4_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::HATCHING,
     .frame_count = 2,
     .length = 16,
     .frames_data = TAMA_HATCHING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::PET_SELECTION,
     .frame_count = 1,
     .length = 16,
     .frames_data = TAMA_PET_SELECTION_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::FEED_CONFIRM,
     .frame_count = 1,
     .length = 8,
     .frames_data = TAMA_FEED_CONFIRM_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::FEEDING,
     .frame_count = 5,
     .length = 8,
     .frames_data = TAMA_FEEDING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::TRAINING_CONFIRM,
     .frame_count = 1,
     .length = 8,
     .frames_data = TAMA_TRAINING_CONFIRM_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::DOG_FED_HEALING,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_DOG_FED_HEALING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT_FED_HEALING,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_CAT_FED_HEALING_FRAMES},

    {.type = TAMA_ANIMATION_TYPE::HEART_3,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_HEART_3_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::HEART_2,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_HEART_2_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::HEART_1,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_HEART_1_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::NEED_HEAL,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_NEED_HEAL_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::LV,
     .frame_count = 2,
     .length = 4,
     .frames_data = TAMA_LV_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_BATTLE_INVITE,
     .frame_count = 1,
     .length = 8,
     .frames_data = TAMA_XB_BATTLE_INVITE_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_PLAYER_DOG,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_PLAYER_DOG_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_PLAYER_CAT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_PLAYER_CAT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_ENEMY_DOG,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_ENEMY_DOG_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_ENEMY_CAT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_ENEMY_CAT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_PLAYER_DOG_HURT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_PLAYER_DOG_HURT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_PLAYER_CAT_HURT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_PLAYER_CAT_HURT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_ENEMY_DOG_HURT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_ENEMY_DOG_HURT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::XB_ENEMY_CAT_HURT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_XB_ENEMY_CAT_HURT_FRAMES},
};

// Macro to check animation properties
#define ASSERT_ANIMATION_PROPERTIES(TYPE_NAME_STR)                        \
  static_assert(                                                          \
      animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)] \
              .type == TAMA_ANIMATION_TYPE::TYPE_NAME_STR,                \
      #TYPE_NAME_STR " Animation type mismatch");                         \
  static_assert(                                                          \
      animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)] \
              .frame_count ==                                             \
          (sizeof(TAMA_##TYPE_NAME_STR##_FRAMES) /                        \
           sizeof(display_buf_t                                           \
                      [animation[static_cast<uint8_t>(                    \
                                     TAMA_ANIMATION_TYPE::TYPE_NAME_STR)] \
                           .length])),                                    \
      #TYPE_NAME_STR " Frame count mismatch");

// Using the macro for static asserts
ASSERT_ANIMATION_PROPERTIES(DOG_IDLE);
ASSERT_ANIMATION_PROPERTIES(CAT_IDLE);
ASSERT_ANIMATION_PROPERTIES(DOG_WEAK);
ASSERT_ANIMATION_PROPERTIES(CAT_WEAK);
ASSERT_ANIMATION_PROPERTIES(EGG_1);
ASSERT_ANIMATION_PROPERTIES(EGG_2);
ASSERT_ANIMATION_PROPERTIES(EGG_3);
ASSERT_ANIMATION_PROPERTIES(EGG_4);
ASSERT_ANIMATION_PROPERTIES(HATCHING);
ASSERT_ANIMATION_PROPERTIES(PET_SELECTION);
ASSERT_ANIMATION_PROPERTIES(FEED_CONFIRM);
ASSERT_ANIMATION_PROPERTIES(FEEDING);
ASSERT_ANIMATION_PROPERTIES(TRAINING_CONFIRM);
ASSERT_ANIMATION_PROPERTIES(DOG_FED_HEALING);
ASSERT_ANIMATION_PROPERTIES(CAT_FED_HEALING);
ASSERT_ANIMATION_PROPERTIES(HEART_3);
ASSERT_ANIMATION_PROPERTIES(HEART_2);
ASSERT_ANIMATION_PROPERTIES(HEART_1);
ASSERT_ANIMATION_PROPERTIES(NEED_HEAL);
ASSERT_ANIMATION_PROPERTIES(LV);
ASSERT_ANIMATION_PROPERTIES(XB_BATTLE_INVITE);
ASSERT_ANIMATION_PROPERTIES(XB_PLAYER_DOG);
ASSERT_ANIMATION_PROPERTIES(XB_PLAYER_CAT);
ASSERT_ANIMATION_PROPERTIES(XB_ENEMY_DOG);
ASSERT_ANIMATION_PROPERTIES(XB_ENEMY_CAT);
ASSERT_ANIMATION_PROPERTIES(XB_PLAYER_DOG_HURT);
ASSERT_ANIMATION_PROPERTIES(XB_PLAYER_CAT_HURT);
ASSERT_ANIMATION_PROPERTIES(XB_ENEMY_DOG_HURT);
ASSERT_ANIMATION_PROPERTIES(XB_ENEMY_CAT_HURT);

// --- Display Component ---
// The data here is used to stack upon existing frames
// clang-format off
constexpr display_buf_t TAMA_PET_SELECTION_CURSOR[8] = {
  0x82, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x82};
constexpr display_buf_t TAMA_N_FONT[3] = { 0b00111100, 0b00000100, 0b00111000 };
constexpr display_buf_t TAMA_Y_FONT[3] = { 0b01011100, 0b01010000, 0b00111100 };
constexpr display_buf_t TAMA_HOSPITAL_ICONS[8] = {
  0x00, 0x18, 0x18, 0x7E, 0x7E, 0x18, 0x18, 0x00};
constexpr display_buf_t TAMA_SELECTION_CURSOR[3] = {0x80, 0x80, 0x80};
constexpr display_buf_t TAMA_NUM_ONE[3] = {0, 0, 0b11111000};
constexpr display_buf_t TAMA_NUM_TWO[3] = {0b11101000, 0b10101000, 0b10111000};
constexpr display_buf_t TAMA_NUM_THREE[3] = {0b10101000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_FOUR[3] = {0b00111000, 0b00100000, 0b11111000};
constexpr display_buf_t TAMA_NUM_FIVE[3] = {0b10111000, 0b10101000, 0b11101000};
constexpr display_buf_t TAMA_NUM_SIX[3] = {0b11111000, 0b10101000, 0b11101000};
constexpr display_buf_t TAMA_NUM_SEVEN[3] = {0b00001000, 0b00001000, 0b11111000};
constexpr display_buf_t TAMA_NUM_EIGHT[3] = {0b11111000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_NINE[3] = {0b10111000, 0b10101000, 0b11111000};
constexpr display_buf_t TAMA_NUM_ZERO[3] = {0b11111000, 0b10001000, 0b11111000};
constexpr display_buf_t TAMA_QTE_WINNING_EFFECT[15] = {0x02, 0x04, 0x08, 0x01, 0x02, 0x04, 0x00, 0x03, 0x00, 0x04, 0x02, 0x01, 0x08, 0x04, 0x02};
constexpr display_buf_t TAMA_QTE_LOSING_EFFECT[11] = {0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04};
constexpr display_buf_t TAMA_TRAINING_FACILITY[6] = {0x26, 0x39, 0x26, 0x4C, 0x72, 0x4C};
constexpr display_buf_t TAMA_TRAINING_LV_UP[12] = {0x7, 0x4, 0, 0x3, 0x4, 0x3, 0, 0x2, 0x7, 0x2, 0, 0x7};
// clang-format on

constexpr tama_display_component_t TAMA_COMPONENT_PET_SELECTION_CURSOR = {
    .data = TAMA_PET_SELECTION_CURSOR,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_N_FONT = {
    .data = TAMA_N_FONT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_Y_FONT = {
    .data = TAMA_Y_FONT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_SELECTION_CURSOR = {
    .data = TAMA_SELECTION_CURSOR,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_HOSPITAL_ICONS = {
    .data = TAMA_HOSPITAL_ICONS,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_ONE = {
    .data = TAMA_NUM_ONE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_TWO = {
    .data = TAMA_NUM_TWO,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_THREE = {
    .data = TAMA_NUM_THREE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_FOUR = {
    .data = TAMA_NUM_FOUR,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_FIVE = {
    .data = TAMA_NUM_FIVE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_SIX = {
    .data = TAMA_NUM_SIX,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_SEVEN = {
    .data = TAMA_NUM_SEVEN,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_EIGHT = {
    .data = TAMA_NUM_EIGHT,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_NINE = {
    .data = TAMA_NUM_NINE,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_NUM_ZERO = {
    .data = TAMA_NUM_ZERO,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_QTE_WINNING_EFFECT = {
    .data = TAMA_QTE_WINNING_EFFECT,
    .length = 15,
};
constexpr tama_display_component_t TAMA_COMPONENT_QTE_LOSING_EFFECT = {
    .data = TAMA_QTE_LOSING_EFFECT,
    .length = 11,
};
constexpr tama_display_component_t TAMA_COMPONENT_TRAINING_FACILITY = {
    .data = TAMA_TRAINING_FACILITY,
    .length = 6,
};
constexpr tama_display_component_t TAMA_COMPONENT_TRAINING_LV_UP = {
    .data = TAMA_TRAINING_LV_UP,
    .length = 12,
};

#define ASSERT_COMPONENT_PROPERTIES(TYPE_NAME_STR)                             \
  static_assert(TAMA_COMPONENT_##TYPE_NAME_STR.length ==                       \
                    sizeof(TAMA_##TYPE_NAME_STR) / sizeof(display_buf_t),      \
                #TYPE_NAME_STR " Frame count mismatch");                       \
  static_assert((TAMA_COMPONENT_##TYPE_NAME_STR).data == TAMA_##TYPE_NAME_STR, \
                #TYPE_NAME_STR " Data mismatch");

ASSERT_COMPONENT_PROPERTIES(PET_SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(N_FONT);
ASSERT_COMPONENT_PROPERTIES(Y_FONT);
ASSERT_COMPONENT_PROPERTIES(SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(HOSPITAL_ICONS);
ASSERT_COMPONENT_PROPERTIES(NUM_ONE);
ASSERT_COMPONENT_PROPERTIES(NUM_TWO);
ASSERT_COMPONENT_PROPERTIES(NUM_THREE);
ASSERT_COMPONENT_PROPERTIES(NUM_FOUR);
ASSERT_COMPONENT_PROPERTIES(NUM_FIVE);
ASSERT_COMPONENT_PROPERTIES(NUM_SIX);
ASSERT_COMPONENT_PROPERTIES(NUM_SEVEN);
ASSERT_COMPONENT_PROPERTIES(NUM_EIGHT);
ASSERT_COMPONENT_PROPERTIES(NUM_NINE);
ASSERT_COMPONENT_PROPERTIES(NUM_ZERO);
ASSERT_COMPONENT_PROPERTIES(QTE_WINNING_EFFECT);
ASSERT_COMPONENT_PROPERTIES(QTE_LOSING_EFFECT)
ASSERT_COMPONENT_PROPERTIES(TRAINING_FACILITY);
ASSERT_COMPONENT_PROPERTIES(TRAINING_LV_UP);

constexpr tama_display_component_t TAMA_NUM_FONT[10] = {
    TAMA_COMPONENT_NUM_ZERO,  TAMA_COMPONENT_NUM_ONE,
    TAMA_COMPONENT_NUM_TWO,   TAMA_COMPONENT_NUM_THREE,
    TAMA_COMPONENT_NUM_FOUR,  TAMA_COMPONENT_NUM_FIVE,
    TAMA_COMPONENT_NUM_SIX,   TAMA_COMPONENT_NUM_SEVEN,
    TAMA_COMPONENT_NUM_EIGHT, TAMA_COMPONENT_NUM_NINE,
};

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#endif  // TAMA_APP_H
