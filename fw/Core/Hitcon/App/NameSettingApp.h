#include <App/EditNameApp.h>
#include <App/NameDisplayApp.h>
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

constexpr menu_entry_t name_setting_menu_entries[] = {
    {"EditName", &edit_name_app, nullptr},
    {"NameDisplay", &name_display_menu, nullptr},
};
constexpr int name_setting_menu_entries_len =
    sizeof(name_setting_menu_entries) / sizeof(menu_entry_t);

class NameSettingApp : public MenuApp {
 public:
  NameSettingApp()
      : MenuApp(name_setting_menu_entries, name_setting_menu_entries_len) {}
  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
  void OnButtonBack() override { badge_controller.change_app(&show_name_app); }
  void OnButtonLongBack() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern NameSettingApp name_setting_menu;

}  // namespace hitcon