#include "ShowScoreApp.h"

#include <App/MainMenuApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Util/uint_to_str.h>

#include <cstring>

namespace hitcon {

static constexpr char PREFIX[] = "Score:";
static constexpr int PREFIX_LEN = sizeof(PREFIX) / sizeof(char);
ShowScoreApp show_score_app;

ShowScoreApp::ShowScoreApp() : score(0) {}

void ShowScoreApp::Init() {}

void ShowScoreApp::OnEntry() { ShowScore(); }

void ShowScoreApp::OnExit() {}

void ShowScoreApp::SetScore(uint32_t score) { this->score = score; }

void ShowScoreApp::ShowScore() {
  char num_str[SCORE_LEN + 1];
  strncpy(num_str, PREFIX, PREFIX_LEN);
  uint_to_chr(num_str + PREFIX_LEN - 1, SCORE_LEN - PREFIX_LEN, score);
  num_str[SCORE_LEN] = '\0';
  display_set_mode_scroll_text(num_str);
}

void ShowScoreApp::OnButton(button_t button) {
  badge_controller.BackToMenu(this);
}

}  // namespace hitcon