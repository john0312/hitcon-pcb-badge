#include "NameDisplayApp.h"

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

menu_entry_t name_display_menu_entries[] = {
    {"Name+Score", nullptr, &ChangeToNameScore},
    {"Name Only", nullptr, &ChangeToNameOnly},
    {"Score Only", nullptr, &ChangeToScoreOnly},
};

NameDisplayApp name_display_menu;

}  // namespace hitcon