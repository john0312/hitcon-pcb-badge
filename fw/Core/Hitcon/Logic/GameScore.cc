#include <Logic/GameLogic.h>
#include <Logic/GameScore.h>
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
    {.type = GameScoreType::GAME_TETRIS, .min_score = 10, .data_row = 0},
    {.type = GameScoreType::GAME_TETRIS, .min_score = 10, .data_row = 0}};

constexpr size_t kAchievementCount =
    sizeof(kAchievementEntries) / sizeof(kAchievementEntries[0]);

constexpr size_t kAchievementPerPass = 16;

constexpr int kIdleRoutineDelay = 35;
constexpr int kBusyRoutineDelay = 100;

}  // namespace

GameScore::GameScore()
    : routine_task_delayed(920, (callback_t)&GameScore::Routine, this, 0) {}

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
  scores[static_cast<int>(game_type)] = score;
}

}  // namespace hitcon
