#ifndef TAMA_APP_H
#define TAMA_APP_H
#define TAMA_APP_MAX_FB_LENGTH 12
#define TAMA_HATCHING_STEPS 400

#define TAMA_PREPARE_FB(FB, FB_SIZE) \
  FB.fb_size = FB_SIZE;              \
  memset(FB.fb, 0, sizeof(DISPLAY_WIDTH) * FB_SIZE);
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
  HP_DETAIL,
  FD_DETAIL,
  LV_DETAIL,
  FEED_CONFIRM,
  FEED_ANIME,
  PET_FED,
  DOG_HEALING,
  CAT_HEALING,
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
  bool hatched;
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
  FEEDING,
  DOG_FED_HEALING,
  CAT_FED_HEALING,
  FEED_CONFIRM,
  HEART_3,
  HEART_2,
  HEART_1,
};

typedef struct {
  TAMA_XBOARD_PACKET_TYPE packet_type;
  uint8_t user[hitcon::ir::IR_USERNAME_LEN];
  uint8_t score;
  uint8_t nonce;
} __attribute__((__packed__)) tama_xboard_result_t;

constexpr uint8_t QTE_REFRESH_RATE = 50;
constexpr uint16_t PAUSE_BETWEEN_QTE_GAMES = 1000;

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
  void Routine();
  void Render();
  void Exit();

 public:
  TamaQte();
  // Init the qte. This should be called only once on startup.
  void Init();
  // Start the qte game. This should be called every time QTE starts.
  void Entry();
  void OnButton(button_t button);
  bool IsDone();
  uint8_t GetScore();
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
  TAMA_TYPE _current_selection_in_choose_mode;
  hitcon::service::sched::PeriodicTask _routine_task;
  hitcon::service::sched::DelayedTask _hatching_task;
  tama_storage_t& _tama_data;
  tama_display_fb_t _fb;
  int _frame_count = 0;
  int _feeding_anime_frame = 0;
  bool _is_selected = false;
  unsigned int _previous_hatching_step = 0;
  unsigned int _total_hatchin_steps = 0;

  bool _is_display_packed = true;
  TamaQte qte;

  void Render();
  void Routine(void* unused);
  void UpdateFrameBuffer();
  void StackOnFrame(const tama_display_component_t* component, int offset);
  void StackOnFrame(const tama_display_component_t* component,
                    display_buf_t mask, int offset);
  void ConcateAnimtaions(uint8_t count, ...);
  void HatchingRoutine(void* unused);

  // XBoard related
  TAMA_XBOARD_STATE _enemy_state;
  uint8_t _my_nounce;
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
  0x00, 0x70, 0xF8, 0xFC, 0xFC, 0xF8, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_2_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0xB8, 0xFC, 0xF4, 0x78, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x70, 0xB8, 0xFC, 0xF4, 0x78, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_3_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0xA8, 0xFC, 0xE4, 0x58, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x70, 0xA8, 0xFC, 0xE4, 0x58, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
};
constexpr display_buf_t TAMA_EGG_4_FRAMES[] = {
  // size 2x16
  0x00, 0x70, 0x28, 0x0C, 0xA0, 0x98, 0x70, 0x00, 0x00, 0b00111100, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
  0x00, 0x70, 0x28, 0x0C, 0xA0, 0x98, 0x70, 0x00, 0x00, 0b00111100, 0b00100100, 0b00100100, 0b00100100, 0b00100100, 0b00111100, 0x00,
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
constexpr display_buf_t TAMA_FEEDING_FRAMES[] = {
  // size 4x7
  0x38, 0x5C, 0xF6, 0xBE, 0xFA, 0x6C, 0x38,
  0x38, 0x54, 0xF0, 0xAA, 0xF0, 0x68, 0x30,
  0x20, 0x50, 0xA0, 0x48, 0x80, 0x50, 0x00,
  0, 0, 0, 0, 0, 0, 0,
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
constexpr display_buf_t TAMA_FEED_CONFIRM_FRAMES[] = {
  // size 1x16
  0x3C, 0x08, 0x10, 0x3C, 0x00, 0x38, 0x5C, 0xF6, 0xBE, 0xFA, 0x6C, 0x38, 0x00, 0x1C, 0x30, 0x1C
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
    {.type = TAMA_ANIMATION_TYPE::FEEDING,
     .frame_count = 4,
     .length = 7,
     .frames_data = TAMA_FEEDING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::DOG_FED_HEALING,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_DOG_FED_HEALING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT_FED_HEALING,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_CAT_FED_HEALING_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::FEED_CONFIRM,
     .frame_count = 1,
     .length = 16,
     .frames_data = TAMA_FEED_CONFIRM_FRAMES},
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
ASSERT_ANIMATION_PROPERTIES(FEEDING);
ASSERT_ANIMATION_PROPERTIES(DOG_FED_HEALING);
ASSERT_ANIMATION_PROPERTIES(CAT_FED_HEALING);
ASSERT_ANIMATION_PROPERTIES(FEED_CONFIRM);
ASSERT_ANIMATION_PROPERTIES(HEART_3);
ASSERT_ANIMATION_PROPERTIES(HEART_2);
ASSERT_ANIMATION_PROPERTIES(HEART_1);

// --- Display Component ---
// The data here is used to stack upon existing frames
// clang-format off
constexpr display_buf_t TAMA_PET_SELECTION_CURSOR[8] = {
  0x82, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x82};
constexpr display_buf_t TAMA_HP_FOOD_ICONS[3] = {0b01000110, 0b10101100,
                                                 0b01000110};
constexpr display_buf_t TAMA_HOSPITAL_ICONS[8] = {
  0x00, 0x18, 0x18, 0x7E, 0x7E, 0x18, 0x18, 0x00};
constexpr display_buf_t TAMA_N_SELECTION_CURSOR[4] = {0x80, 0x80, 0x80, 0x80};
constexpr display_buf_t TAMA_Y_SELECTION_CURSOR[3] = {0x80, 0x80, 0x80};
// clang-format on

constexpr tama_display_component_t TAMA_COMPONENT_PET_SELECTION_CURSOR = {
    .data = TAMA_PET_SELECTION_CURSOR,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_HP_FOOD_ICONS = {
    .data = TAMA_HP_FOOD_ICONS,
    .length = 3,
};
constexpr tama_display_component_t TAMA_COMPONENT_HOSPITAL_ICONS = {
    .data = TAMA_HOSPITAL_ICONS,
    .length = 8,
};
constexpr tama_display_component_t TAMA_COMPONENT_N_SELECTION_CURSOR = {
    .data = TAMA_N_SELECTION_CURSOR,
    .length = 4,
};
constexpr tama_display_component_t TAMA_COMPONENT_Y_SELECTION_CURSOR = {
    .data = TAMA_Y_SELECTION_CURSOR,
    .length = 3,
};

#define ASSERT_COMPONENT_PROPERTIES(TYPE_NAME_STR)                             \
  static_assert(TAMA_COMPONENT_##TYPE_NAME_STR.length ==                       \
                    sizeof(TAMA_##TYPE_NAME_STR) / sizeof(display_buf_t),      \
                #TYPE_NAME_STR " Frame count mismatch");                       \
  static_assert((TAMA_COMPONENT_##TYPE_NAME_STR).data == TAMA_##TYPE_NAME_STR, \
                #TYPE_NAME_STR " Data mismatch");

ASSERT_COMPONENT_PROPERTIES(PET_SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(HP_FOOD_ICONS);
ASSERT_COMPONENT_PROPERTIES(HOSPITAL_ICONS);
ASSERT_COMPONENT_PROPERTIES(N_SELECTION_CURSOR);
ASSERT_COMPONENT_PROPERTIES(Y_SELECTION_CURSOR);

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#endif  // TAMA_APP_H
