#include "MenuApp.h"

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>

namespace hitcon {

extern menu_entry_t name_display_menu_entries[];

class NameDisplayApp : public MenuApp {
 public:
  NameDisplayApp() : MenuApp(name_display_menu_entries) {}
  void OnButtonMode() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern NameDisplayApp name_display_menu;

} // namespace hitcon