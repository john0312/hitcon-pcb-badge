#ifndef LOGIC_GAME_SCORE_H_
#define LOGIC_GAME_SCORE_H_

#include <Secret/secret.h>

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

 private:
  uint8_t sent[kGameAchievementDataCount / 8 + 1];
  int scores[static_cast<size_t>(GameScoreType::GAME_UNUSED_MAX)];

  void Routine(void* args);
  bool RoutineInternal();
};

extern GameScore g_game_score;

}  // namespace hitcon

#endif  // LOGIC_GAME_SCORE_H_
