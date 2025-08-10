#ifndef LOGIC_GAME_SCORE_H_
#define LOGIC_GAME_SCORE_H_

#include <Service/Sched/Scheduler.h>

namespace hitcon {

enum class GameScoreType {
  GAME_TETRIS = 0,
  GAME_SNAKE,
  GAME_DINO,
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
