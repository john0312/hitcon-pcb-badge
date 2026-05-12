#ifndef LOGIC_GAME_SCORE_H_
#define LOGIC_GAME_SCORE_H_

#include <Service/Sched/Scheduler.h>

namespace hitcon {

// Each entry's numeric value is a slot index in NvStorage's max_scores[].
// Append-only: never reorder, renumber, or delete middle entries -- doing so
// would silently remap stored high scores to different games. New entries go
// before GAME_UNUSED_MAX. Capacity is bounded by NV_MAX_SCORE_SLOTS in
// NvStorage.h; exceeding it triggers a compile-time assert there.
enum class GameScoreType {
  GAME_TETRIS = 0,
  GAME_SNAKE,
  GAME_DINO,
  GAME_SPACESHIP,
  GAME_UNUSED_MAX
};

// This class keeps track of the game scores.
class GameScore {
 public:
  GameScore();

  void Init();

  void MarkScore(GameScoreType game_type, int score);

  int GetScore(GameScoreType game_type);

 private:
  int scores[static_cast<size_t>(GameScoreType::GAME_UNUSED_MAX)];

  hitcon::service::sched::DelayedTask routine_task_delayed;

  size_t last_operation_progress_;
  bool nv_fetched_;

  void Routine(void* args);
};

extern GameScore g_game_score;

}  // namespace hitcon

#endif  // LOGIC_GAME_SCORE_H_
