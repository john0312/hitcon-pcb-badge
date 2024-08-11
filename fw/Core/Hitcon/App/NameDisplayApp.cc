#include "NameDisplayApp.h"

#include <App/NameSettingApp.h>
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

namespace hitcon {

void ChangeToNameScore() {
  show_name_app.SetMode(NameScore);
  badge_controller.change_app(&show_name_app);
}
void ChangeToNameOnly() {
  show_name_app.SetMode(NameOnly);
  badge_controller.change_app(&show_name_app);
}
void ChangeToScoreOnly() {
  show_name_app.SetMode(ScoreOnly);
  badge_controller.change_app(&show_name_app);
}

void NameDisplayApp::OnButtonMode() {
  badge_controller.change_app(&show_name_app);
}
void NameDisplayApp::OnButtonBack() {
  badge_controller.change_app(&name_setting_menu);
}
void NameDisplayApp::OnButtonLongBack() {
  badge_controller.change_app(&show_name_app);
}

NameDisplayApp name_display_menu;

}  // namespace hitcon