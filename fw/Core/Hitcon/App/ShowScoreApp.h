#ifndef SHOW_SCORE_APP_H
#define SHOW_SCORE_APP_H

#include <Logic/Display/display.h>
#include <Logic/Display/font.h>

#include "app.h"

namespace hitcon {

class ShowScoreApp : public App {
 public:
  static constexpr int SCORE_LEN = kDisplayScrollMaxTextLen;
  uint8_t display_buf[kDisplayScrollMaxTextLen + 1];

  ShowScoreApp();
  virtual ~ShowScoreApp() = default;

  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void SetScore(uint32_t score);

 private:
  void ShowScore();

  uint32_t score = 0;
};

extern ShowScoreApp show_score_app;
}  // namespace hitcon

#endif  // SHOW_SCORE_APP_H
