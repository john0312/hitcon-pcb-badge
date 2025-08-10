#ifndef SHOW_NAME_APP_H
#define SHOW_NAME_APP_H

#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "app.h"

namespace hitcon {

enum ShowNameMode {
  SHOW_INITIALIZE,
  NameScore,
  NameOnly,
  ScoreOnly,
  Surprise,
};

class ShowNameApp : public App {
 public:
  static constexpr int NAME_LEN = kDisplayMaxNameLength;
  static constexpr char *DEFAULT_NAME = "HITCON2025";

  char name[NAME_LEN + 1] = {0};
  char display_buf[DISPLAY_SCROLL_MAX_COLUMNS];

  ShowNameApp();
  virtual ~ShowNameApp() = default;

  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void SetName(const char *name);
  void SetMode(const enum ShowNameMode mode);
  void SetScore(uint32_t score);
  enum ShowNameMode GetMode();

  void SetSurpriseMsg(const char *msg);

  void check_update();

 private:
  enum ShowNameMode mode;
  void update_display();
  hitcon::service::sched::PeriodicTask _routine_task;
  uint32_t score_cache = 0;

  char surprise_msg[kDisplayScrollMaxTextLen + 1];
  bool starting_up;
  unsigned last_disp_update;
};

extern ShowNameApp show_name_app;

}  // namespace hitcon

#endif  // SHOW_NAME_APP
