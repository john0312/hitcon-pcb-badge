#ifndef SHOW_NAME_APP_H
#define SHOW_NAME_APP_H

#include "app.h"
#include <Service/Sched/PeriodicTask.h>
#include <Logic/Display/display.h>

namespace hitcon {

enum ShowNameMode {
  NameScore,
  NameOnly,
  ScoreOnly,
};

class ShowNameApp : public App {
 public:
  static constexpr int NAME_LEN = 16;
  static constexpr char *DEFAULT_NAME = "HITCON2024";
  char name[NAME_LEN + 1] = {0};
  char display_buf[DISPLAY_SCROLL_MAX_COLUMNS];

  ShowNameApp();
  virtual ~ShowNameApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  void SetName(const char *name);
  void SetMode(const enum ShowNameMode mode);
  enum ShowNameMode GetMode();

  void check_update();

 private:
  enum ShowNameMode mode;
  void update_display();
  hitcon::service::sched::PeriodicTask _routine_task;
  uint32_t score_cache = 0;
};

extern ShowNameApp show_name_app;

}  // namespace hitcon

#endif  // SHOW_NAME_APP
