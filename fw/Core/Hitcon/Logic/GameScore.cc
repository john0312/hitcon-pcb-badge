#include <Logic/GameScore.h>

#include <cstdlib>
#include <cstring>

namespace hitcon {

GameScore::GameScore() {}

void GameScore::Init() { memset(sent, 0, sizeof(sent)); }

void GameScore::Routine(void* args) {
  bool idle = RoutineInternal();

  if (idle) {
  } else {
  }
}

void GameScore::MarkScore(GameScoreType game_type, int score) {
  scores[static_cast<int>(game_type)] = score;
}

}  // namespace hitcon
