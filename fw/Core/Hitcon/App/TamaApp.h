#ifndef TAMA_APP_H
#define TAMA_APP_H

#include <Logic/Display/display.h>
#include <Logic/ImuLogic.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/SysTimer.h>  // For SysTimer

#include "app.h"

namespace hitcon {
namespace app {
namespace tama {

enum class TAMA_APP_STATE : uint8_t {
  INTRO_TEXT,   // Displaying introductory text
  CHOOSE_TYPE,  // Player is selecting a pet type
  EGG,          // Pet is in egg state, waiting to hatch
  HATCHING,
  ALIVE,  // Pet has hatched and is alive
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
  unsigned int state_enter_time_ms;  // Timestamp when the current state (e.g.,
  // EGG) was entered
  int hatching_start_shaking_count;
  int latest_shaking_count;
  // TODO: Add more stats for ALIVE state:
  // uint32_t birth_time_ms; // Actual hatch time
  // uint8_t hunger;
  // uint8_t happiness;
  // uint32_t last_interaction_time_ms;
} tama_storage_t;

typedef struct {
  display_buf_t fb[12][DISPLAY_WIDTH];
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
  int anime_frame = 0;

  void Render();
  void Routine(void* unused);
  void UpdateFrameBuffer();

  // XBoard related
  TAMA_XBOARD_STATE _enemy_state;
  uint8_t _qte_count;
  uint8_t _qte_score;
  uint8_t _enemy_score;
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
  void OnEdgeButton(button_t button) override;

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

constexpr display_buf_t TAMA_DOG_FRAMES[2][8] = {
    {
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
    },
    {
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
    }};
constexpr display_buf_t TAMA_CAT_FRAMES[2][8] = {
    {
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
    },
    {
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
    }};
constexpr display_buf_t TAMA_EGG_FRAMES[2][8] = {
    {0x00, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x00},
    {0x00, 0x38, 0x7C, 0xFE, 0xFE, 0x7C, 0x38, 0x00}};
constexpr display_buf_t TAMA_HEART_FRAMES[1][8] = {
    {0b00000000, 0b01100110, 0b11111111, 0b11111111, 0b01111110, 0b00111100,
     0b00011000, 0b00000000}};

// --- Animation Definition Structure --

enum class TAMA_ANIMATION_TYPE : uint8_t { DOG, CAT, EGG, HEART };
typedef struct {
  TAMA_ANIMATION_TYPE type;
  int frame_count;
  const display_buf_t (*frames_data)[8];
} tama_ani_t;

constexpr tama_ani_t animation[] = {
    {.type = TAMA_ANIMATION_TYPE::DOG,
     .frame_count = 2,
     .frames_data = TAMA_DOG_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::CAT,
     .frame_count = 2,
     .frames_data = TAMA_CAT_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::EGG,
     .frame_count = 2,
     .frames_data = TAMA_EGG_FRAMES},
    {.type = TAMA_ANIMATION_TYPE::HEART,
     .frame_count = 1,
     .frames_data = TAMA_HEART_FRAMES},
};

// Macro to check animation properties
#define ASSERT_ANIMATION_PROPERTIES(TYPE_NAME_STR)                            \
  static_assert(                                                              \
      animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)]     \
              .type == TAMA_ANIMATION_TYPE::TYPE_NAME_STR,                    \
      #TYPE_NAME_STR " Animation type mismatch");                             \
  static_assert(                                                              \
      animation[static_cast<uint8_t>(TAMA_ANIMATION_TYPE::TYPE_NAME_STR)]     \
              .frame_count ==                                                 \
          (sizeof(TAMA_##TYPE_NAME_STR##_FRAMES) / sizeof(display_buf_t[8])), \
      #TYPE_NAME_STR " Frame count mismatch")

// Using the macro for static asserts
ASSERT_ANIMATION_PROPERTIES(DOG);
ASSERT_ANIMATION_PROPERTIES(CAT);
ASSERT_ANIMATION_PROPERTIES(EGG);
ASSERT_ANIMATION_PROPERTIES(HEART);

// Left Arrow '<' (2 columns wide, 3 pixels high: rows 3,4,5)
/* Visualized (Row-major, '.' is off, 'X' is on, for the arrow part):
..X. (col 0, row 4)
.X.. (col 1, row 3)
.X.. (col 1, row 5)
*/
constexpr int BITMAP_LEFT_ARROW_WIDTH = 2;
constexpr display_buf_t bitmap_left_arrow_cols[BITMAP_LEFT_ARROW_WIDTH] = {
    0b00010000,  // Column 0: pixel at row 4
    0b00101000   // Column 1: pixels at row 3 and 5
};

// Right Arrow '>' (2 columns wide, 3 pixels high: rows 3,4,5)
/* Visualized (Row-major, '.' is off, 'X' is on, for the arrow part):
.X.. (col 0, row 3)
.X.. (col 0, row 5)
..X. (col 1, row 4)
*/
constexpr int BITMAP_RIGHT_ARROW_WIDTH = 2;
constexpr display_buf_t bitmap_right_arrow_cols[BITMAP_RIGHT_ARROW_WIDTH] = {
    0b00101000,  // Column 0: pixels at row 3 and 5
    0b00010000   // Column 1: pixel at row 4
};

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#endif  // TAMA_APP_H
