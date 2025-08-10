#include "GameScore.h"

#include <Logic/GameScore.h>
#include <Logic/NvStorage.h>
#include <Service/Sched/SysTimer.h>

#include <cstdlib>
#include <cstring>

namespace hitcon {

GameScore g_game_score;

namespace {

using hitcon::service::sched::scheduler;
using hitcon::service::sched::SysTimer;

constexpr int kIdleRoutineDelay = 35;
constexpr int kBusyRoutineDelay = 100;

}  // namespace

GameScore::GameScore()
    : routine_task_delayed(920, (callback_t)&GameScore::Routine, this, 0),
      nv_fetched_(false) {}

void GameScore::Init() {
  last_operation_progress_ = 0;

  routine_task_delayed.SetWakeTime(SysTimer::GetTime() + kIdleRoutineDelay);
  scheduler.Queue(&routine_task_delayed, nullptr);
}

void GameScore::Routine(void* args) {
  // remove implementation
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
