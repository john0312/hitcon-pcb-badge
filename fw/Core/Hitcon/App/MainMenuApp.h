#include "MenuApp.h"

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>


namespace hitcon {

extern menu_entry_t main_menu_entries[];

class MainMenuApp : public MenuApp {
 public:
  MainMenuApp() : MenuApp(main_menu_entries) {}
  void OnButtonMode() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern MainMenuApp main_menu;

} // namespace hitcon