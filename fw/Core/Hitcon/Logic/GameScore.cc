#include "GameScore.h"

#include <Logic/GameLogic.h>
#include <Logic/GameScore.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/SysTimer.h>

#include <cstdlib>
#include <cstring>

namespace hitcon {

GameScore g_game_score;

namespace {

using hitcon::game::gameLogic;
using hitcon::service::sched::scheduler;
using hitcon::service::sched::SysTimer;

typedef struct AchievementEntry_t {
  GameScoreType type;
  int min_score;
  size_t data_row;
} AchievementEntry;

constexpr AchievementEntry kAchievementEntries[]{
    // clang-format off
    /* Tetris */
    {.type = GameScoreType::GAME_TETRIS, .min_score =0x0080, .data_row = 0x00},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x0100, .data_row = 0x01},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x0200, .data_row = 0x02},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x0400, .data_row = 0x03},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x0800, .data_row = 0x04},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x1000, .data_row = 0x05},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x1800, .data_row = 0x06},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x2000, .data_row = 0x07},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x2800, .data_row = 0x08},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x3000, .data_row = 0x09},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x3800, .data_row = 0x0a},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x4000, .data_row = 0x0b},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x5000, .data_row = 0x0c},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x6000, .data_row = 0x0d},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x7000, .data_row = 0x0e},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 0x8000, .data_row = 0x0f},

    /* Dino */
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0020, .data_row = 0x10},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0040, .data_row = 0x11},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0060, .data_row = 0x12},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0080, .data_row = 0x13},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x00a0, .data_row = 0x14},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0100, .data_row = 0x15},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0180, .data_row = 0x16},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0200, .data_row = 0x17},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0280, .data_row = 0x18},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0300, .data_row = 0x19},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0400, .data_row = 0x1a},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0500, .data_row = 0x1b},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0600, .data_row = 0x1c},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0800, .data_row = 0x1d},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x0a00, .data_row = 0x1e},
    {.type = GameScoreType::GAME_DINO, .min_score = 0x1000, .data_row = 0x1f},

    /* Snake */
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x08, .data_row = 0x20},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x0c, .data_row = 0x21},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x10, .data_row = 0x22},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x14, .data_row = 0x23},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x18, .data_row = 0x24},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x1c, .data_row = 0x25},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x20, .data_row = 0x26},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x24, .data_row = 0x27},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x28, .data_row = 0x28},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x30, .data_row = 0x29},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x38, .data_row = 0x2a},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x40, .data_row = 0x2b},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x48, .data_row = 0x2c},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x50, .data_row = 0x2d},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x58, .data_row = 0x2e},
    {.type = GameScoreType::GAME_SNAKE, .min_score = 0x7f, .data_row = 0x2f},
    // clang-format on
};

constexpr size_t kAchievementCount =
    sizeof(kAchievementEntries) / sizeof(kAchievementEntries[0]);

constexpr size_t kAchievementPerPass = 16;

constexpr int kIdleRoutineDelay = 35;
constexpr int kBusyRoutineDelay = 100;

}  // namespace

GameScore::GameScore()
    : routine_task_delayed(920, (callback_t)&GameScore::Routine, this, 0),
      nv_fetched_(false) {}

void GameScore::Init() {
  memset(sent, 0, sizeof(sent));
  last_operation_progress_ = 0;

  routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kIdleRoutineDelay);
  scheduler.Queue(&routine_task_delayed, nullptr);
}

void GameScore::Routine(void* args) {
  bool idle = RoutineInternal();

  if (idle) {
    routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kIdleRoutineDelay);
    scheduler.Queue(&routine_task_delayed, nullptr);
  } else {
    routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kBusyRoutineDelay);
    scheduler.Queue(&routine_task_delayed, nullptr);
  }
}

bool GameScore::RoutineInternal() {
  if (g_nv_storage.IsStorageValid() && !nv_fetched_) {
    for (size_t i = 0; i < static_cast<size_t>(GameScoreType::GAME_UNUSED_MAX);
         i++) {
      if (g_nv_storage.GetCurrentStorage().max_scores[i] > scores[i]) {
        scores[i] = g_nv_storage.GetCurrentStorage().max_scores[i];
      }
    }
    nv_fetched_ = true;
  }
  size_t i = last_operation_progress_;
  size_t end = kAchievementCount;
  if (kAchievementPerPass < end) end = kAchievementPerPass;
  for (; i < end; i++) {
    if (scores[static_cast<size_t>(kAchievementEntries[i].type)] >=
        kAchievementEntries[i].min_score) {
      // Need to accept.
      if (!sent[i / 8] & (1 << (i % 8))) {
        bool ret = TryAcceptData(kAchievementEntries[i].data_row);
        if (ret) {
          sent[i / 8] |= (1 << (i % 8));
        } else {
          break;
        }
      }
    }
  }
  if (i >= kAchievementCount) {
    last_operation_progress_ = 0;
    return true;
  } else {
    last_operation_progress_ = i;
    return false;
  }
}

bool GameScore::TryAcceptData(size_t row_id) {
  int col = static_cast<int>(kGameAchievementData[row_id * 9 + 0]);
  uint8_t data[8];
  memcpy(data, &kGameAchievementData[row_id * 9 + 1],
         ::hitcon::game::kDataSize);
  return gameLogic.AcceptData(col, data);
}

void GameScore::MarkScore(GameScoreType game_type, int score) {
  const size_t sid = static_cast<int>(game_type);
  if (score > scores[sid]) {
    scores[sid] = score;
    g_nv_storage.GetCurrentStorage().max_scores[sid] = score;
    g_nv_storage.MarkDirty();
  }
}

int GameScore::GetScore(GameScoreType game_type) {
  const size_t sid = static_cast<int>(game_type);
  return scores[sid];
}

}  // namespace hitcon
