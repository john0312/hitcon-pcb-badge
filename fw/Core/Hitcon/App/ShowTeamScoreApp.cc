#include "ShowTeamScoreApp.h"

#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Service/Sched/Scheduler.h>
namespace hitcon {
namespace app {
namespace team_score {

using namespace hitcon::service::sched;
using namespace hitcon::logic::cdc;

ShowTeamScoreApp::ShowTeamScoreApp()
    : routine_task(490, (task_callback_t)&ShowTeamScoreApp::CheckUpdate, this,
                   1000) {}

void ShowTeamScoreApp::Init() {
  scheduler.Queue(&routine_task, nullptr);
  g_cdc_logic.SetOnPacketArrive(
      (callback_t)&ShowTeamScoreApp::SetDisplayHandler, this,
      FnId::SetDisplayData);
}

void ShowTeamScoreApp::OnEntry() {
  scheduler.EnablePeriodic(&routine_task);
  UpdateDisplay();
}

void ShowTeamScoreApp::OnExit() { scheduler.DisablePeriodic(&routine_task); }

void ShowTeamScoreApp::OnButton(button_t button) {}

// private
void ShowTeamScoreApp::CheckUpdate() {
  if (need_updated) {
    UpdateDisplay();
  }
}

void ShowTeamScoreApp::UpdateDisplay() {
  if (!have_display_data) {
    display_set_mode_scroll_text("INIT");
  } else {
    display_set_mode_fixed_packed(display_data.data());
  }
  need_updated = false;
}

void ShowTeamScoreApp::SetDisplayHandler(PacketCallbackArg* arg) {
  if (arg->len != 16) {
    return;  // Invalid data length
  }
  have_display_data = true;
  need_updated = true;
  std::copy(arg->data, arg->data + arg->len, display_data.begin());
  if (badge_controller.GetCurrentApp() == this) {
    // Only update display if this app is currently active
    UpdateDisplay();
  }
}

ShowTeamScoreApp show_team_score_app;

}  // namespace team_score
}  // namespace app
}  // namespace hitcon
