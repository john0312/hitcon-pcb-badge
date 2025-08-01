#ifndef TAMA_APP_H
#define TAMA_APP_H
#define TAMA_APP_MAX_FB_LENGTH 12

#define TAMA_PREPARE_FB(FB, FB_SIZE) \
  FB.fb_size = FB_SIZE;              \
  memset(FB.fb, 0, sizeof(DISPLAY_WIDTH) * FB_SIZE);
#define TAMA_GET_ANIMATION_DATA(TYPE_NAME_STR) \
  animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)]
#define TAMA_COPY_FB(FB, ANIMATION, OFFSET)                      \
  FB.active_frame = 0;                                           \
  FB.fb_size = ANIMATION.frame_count;                            \
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
  ALIVE,
  HP_DETAIL,
  FD_DETAIL,
  LV_DETAIL,
  FEED_CONFIRM,
  FEED_ANIME,
  // TODO: Add states like DEAD, EVOLVING, etc.
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
  uint8_t food;
  uint8_t hp;

  // TODO: Add more stats for ALIVE state:
  // uint32_t birth_time_ms; // Actual hatch time

  // uint8_t happiness;
  // uint32_t last_interaction_time_ms;
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
  DOG,
  CAT,
  EGG_1,
  EGG_2,
  EGG_3,
  EGG_4,
  HATCHING,
  PET_SELECTION
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
  tama_storage_t& _tama_data;
  tama_display_fb_t _fb;
  int _frame_count = 0;
  int hatching_warning_frame_count = 10;  // How many times the egg has shined
  int _feeding_anime_frame = 0;
  int anime_frame = 0;
  bool _is_selected = false;                 // For testing, use temp storage
  unsigned int _previous_hatching_step = 0;  // Will update every 100 steps
  bool _is_display_packed = true;
  TamaQte qte;

  void Render();
  void Routine(void* unused);
  void UpdateFrameBuffer();
  void StackOnFrame(const tama_display_component_t* component, int offset);
  void ConcateAnimtaions(uint8_t count, tama_ani_t** animations);

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
constexpr display_buf_t TAMA_DOG_FRAMES[] = {
    // size 2*8
    /*
     * code=68, hex=0x44, ascii="D"
     */
    0xf0, /* 111100 */
    0x88, /* 100010 */
    0x88, /* 100010 */
    0x88, /* 100010 */
    0x88, /* 100010 */
    0x88, /* 100010 */
    0xf0, /* 111100 */
    0x00, /* 000000 */
    /*
     * code=71, hex=0x47, ascii="G"
     */
    0x70, /* 011100 */
    0x88, /* 100010 */
    0x80, /* 100000 */
    0x98, /* 100110 */
    0x88, /* 100010 */
    0x88, /* 100010 */
    0x70, /* 011100 */
    0x00, /* 000000 */
};
constexpr display_buf_t TAMA_CAT_FRAMES[] = {
    // size 2*8
    /*
     * code=67, hex=0x43, ascii="C"
     */
    0x70, /* 011100 */
    0x88, /* 100010 */
    0x80, /* 100000 */
    0x80, /* 100000 */
    0x80, /* 100000 */
    0x88, /* 100010 */
    0x70, /* 011100 */
    0x00, /* 000000 */
    /*
     * code=84, hex=0x54, ascii="T"
     */
    0xf8, /* 111110 */
    0x20, /* 001000 */
    0x20, /* 001000 */
    0x20, /* 001000 */
    0x20, /* 001000 */
    0x20, /* 001000 */
    0x20, /* 001000 */
    0x00, /* 000000 */
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
// clang-format on

// --- Animation Definition Structure --

constexpr tama_ani_t animation[] = {
    {.type = TAMA_ANIMATION_TYPE::DOG,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_DOG_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT,
     .frame_count = 2,
     .length = 8,
     .frames_data = TAMA_CAT_FRAMES},
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
      #TYPE_NAME_STR " Frame count mismatch")

// Using the macro for static asserts
ASSERT_ANIMATION_PROPERTIES(DOG);
ASSERT_ANIMATION_PROPERTIES(CAT);
ASSERT_ANIMATION_PROPERTIES(EGG_1);
ASSERT_ANIMATION_PROPERTIES(EGG_2);
ASSERT_ANIMATION_PROPERTIES(EGG_3);
ASSERT_ANIMATION_PROPERTIES(EGG_4);
ASSERT_ANIMATION_PROPERTIES(PET_SELECTION);

// --- Display Component ---
// The data here is used to stack upon existing frames
constexpr display_buf_t TAMA_PET_SELECTION_CURSOR[8] = {0x82, 0x81, 0x81, 0x81,
                                                        0x81, 0x81, 0x81, 0x82};
constexpr tama_display_component_t TAMA_COMPONENT_PET_SELECTION_CURSOR = {
    .data = TAMA_PET_SELECTION_CURSOR,
    .length = 8,
};

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#endif  // TAMA_APP_H
