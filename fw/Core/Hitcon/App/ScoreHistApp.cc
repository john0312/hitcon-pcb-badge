#include <App/ScoreHistApp.h>
#include <Logic/GameScore.h>
#include <Util/uint_to_str.h>

namespace hitcon {
namespace score_hist {

char dino_score_str[15] = "Dino ";
char snake_score_str[15] = "Snake ";
char tetris_score_str[15] = "Tetris ";

ScoreHistApp g_score_hist;

void ScoreHistApp::SetScores() {
  int dino_score = g_game_score.GetScore(GameScoreType::GAME_DINO);
  int snake_score = g_game_score.GetScore(GameScoreType::GAME_SNAKE);
  int tetris_score = g_game_score.GetScore(GameScoreType::GAME_TETRIS);

  if (dino_score > 9999999) dino_score = 9999999;
  if (snake_score > 9999999) snake_score = 9999999;
  if (tetris_score > 9999999) tetris_score = 9999999;

  uint_to_chr(&dino_score_str[5], 8, dino_score);
  uint_to_chr(&tetris_score_str[7], 8, tetris_score);
  uint_to_chr(&snake_score_str[6], 8, snake_score);
}

void ScoreHistApp::OnEntry() {
  SetScores();
  MenuApp::OnEntry();
}

}  // namespace score_hist
}  // namespace hitcon
