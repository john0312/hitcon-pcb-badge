#ifndef HITCON_APP_SHOW_TEAM_SCORE_APP_H
#define HITCON_APP_SHOW_TEAM_SCORE_APP_H

#include <Logic/CdcLogic.h>
#include <Service/Sched/PeriodicTask.h>

#include <array>

#include "app.h"

namespace hitcon {
namespace app {
namespace team_score {

class ShowTeamScoreApp : public App {
 public:
  ShowTeamScoreApp();
  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  bool have_display_data = false;
  bool need_updated = false;
  // time in seconds
  unsigned last_update_time = 0;
  std::array<uint8_t, 16> display_data;
  hitcon::service::sched::PeriodicTask routine_task;
  void CheckUpdate();
  void UpdateDisplay();
  void SetDisplayHandler(hitcon::logic::cdc::PacketCallbackArg* arg);
  bool DataOutdated();
};

extern ShowTeamScoreApp show_team_score_app;

}  // namespace team_score
}  // namespace app
}  // namespace hitcon
#endif  // HITCON_APP_SHOW_TEAM_SCORE_APP_H