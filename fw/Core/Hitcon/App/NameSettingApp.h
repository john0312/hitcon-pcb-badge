#include "MenuApp.h"

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

namespace hitcon {

extern menu_entry_t name_setting_menu_entries[];

class NameSettingApp : public MenuApp {
 public:
  NameSettingApp() : MenuApp(name_setting_menu_entries) {}
  void OnButtonMode() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern NameSettingApp name_setting_menu;

} // namespace hitcon