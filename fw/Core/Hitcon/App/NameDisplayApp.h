#ifndef _HITCON_NAME_DISPLAY_APP_H
#define _HITCON_NAME_DISPLAY_APP_H

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Secret/secret.h>

#include "MenuApp.h"

namespace hitcon {

void ChangeToNameScore();
void ChangeToNameOnly();
void ChangeToScoreOnly();

constexpr menu_entry_t name_display_menu_entries[] = {
    {"NameOnly", nullptr, &ChangeToNameOnly},
    {"Both", nullptr, &ChangeToNameScore},
    {"ScoreOnly", nullptr, &ChangeToScoreOnly},
};
constexpr int name_display_menu_entries_len =
    kForceShowNameOnly
        ? 1
        : sizeof(name_display_menu_entries) / sizeof(menu_entry_t);

class NameDisplayApp : public MenuApp {
 public:
  NameDisplayApp()
      : MenuApp(name_display_menu_entries, name_display_menu_entries_len) {}
  void OnButtonMode() override;
  void OnButtonBack() override;
  void OnButtonLongBack() override;
};

extern NameDisplayApp name_display_menu;

}  // namespace hitcon

#endif  // _HITCON_NAME_DISPLAY_APP_H
