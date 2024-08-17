#ifndef APP_SCORE_HIST_APP
#define APP_SCORE_HIST_APP

#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {
namespace score_hist {

extern char dino_score_str[15];
extern char snake_score_str[15];
extern char tetris_score_str[15];

constexpr menu_entry_t score_hist_app_entries[] = {
    {dino_score_str, nullptr, nullptr},
    {tetris_score_str, nullptr, nullptr},
    {snake_score_str, nullptr, nullptr}};

constexpr size_t score_hist_app_entries_len =
    sizeof(score_hist_app_entries) / sizeof(score_hist_app_entries[0]);

class ScoreHistApp : public MenuApp {
 public:
  ScoreHistApp()
      : MenuApp(score_hist_app_entries, score_hist_app_entries_len) {}

  void OnEntry() override;

  void OnButtonMode() override {}
  void OnButtonBack() override { badge_controller.BackToMenu(this); }
  void OnButtonLongBack() override { badge_controller.BackToMenu(this); }

 private:
  void SetScores();
};

extern ScoreHistApp g_score_hist;

}  // namespace score_hist
}  // namespace hitcon

#endif  // APP_SCORE_HIST_APP
