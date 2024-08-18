#include "EditNameApp.h"

#include <App/NameSettingApp.h>
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Service/ButtonService.h>

namespace hitcon {
EditNameApp edit_name_app;

EditNameApp::EditNameApp() {}

void EditNameApp::OnEntry() {
  editor = TextEditorDisplay(show_name_app.name);
  display_set_mode_editor(&editor);
}

void EditNameApp::OnExit() {}

void EditNameApp::OnButton(button_t button) {
  switch (button) {
    case BUTTON_LEFT:
      if (mode_key_down) {
        editor.backspace();
      } else {
        editor.move_cursor_left();
      }
      break;
    case BUTTON_RIGHT:
      editor.move_cursor_right();
      break;
    case BUTTON_UP:
      editor.incr_current_char();
      break;
    case BUTTON_DOWN:
      editor.decr_current_char();
      break;
    case BUTTON_LONG_BACK:
      badge_controller.change_app(&show_name_app);
      break;
    case BUTTON_BACK:
      badge_controller.change_app(&name_setting_menu);
      break;
    case BUTTON_OK:
      // save the name and exit
      show_name_app.SetName(editor.text);
      badge_controller.change_app(&show_name_app);
      break;
  }
}

void EditNameApp::OnEdgeButton(button_t button) {
  switch (button & BUTTON_VALUE_MASK) {
    case BUTTON_MODE:
      if (button & BUTTON_KEYDOWN_BIT) {
        mode_key_down = true;
      } else if (button & BUTTON_KEYUP_BIT) {
        mode_key_down = false;
      }
      break;
  }
}

}  // namespace hitcon
