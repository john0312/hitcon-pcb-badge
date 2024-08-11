#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

void ChangeToNameScore();
void ChangeToNameOnly();
void ChangeToScoreOnly();

constexpr menu_entry_t name_display_menu_entries[] = {
    {"Name+Score", nullptr, &ChangeToNameScore},
    {"NameOnly", nullptr, &ChangeToNameOnly},
    {"ScoreOnly", nullptr, &ChangeToScoreOnly},
};
constexpr int name_display_menu_entries_len =
    sizeof(name_display_menu_entries) / sizeof(menu_entry_t);

class NameDisplayApp : public MenuApp {
 public:
  NameDisplayApp()
      : MenuApp(name_display_menu_entries, name_display_menu_entries_len) {}
  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
  void OnButtonBack() override { badge_controller.change_app(&name_setting_menu); }
  void OnButtonLongBack() override { badge_controller.change_app(&show_name_app); }
};

extern NameDisplayApp name_display_menu;

}  // namespace hitcon