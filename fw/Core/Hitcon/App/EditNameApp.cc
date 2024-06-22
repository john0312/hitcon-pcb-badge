#include "EditNameApp.h"
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Service/ButtonService.h>

namespace hitcon {

EditNameApp::EditNameApp() {}

void EditNameApp::OnEntry() {
  editor = TextEditorDisplay(show_name_app.name);
  display_set_mode_editor(&editor);
}

void EditNameApp::OnExit() {}

void EditNameApp::OnButton(button_t button) {
  switch (button) {
  case BUTTON_LEFT:
    editor.move_cursor_left();
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
  case BUTTON_BACK:
    badge_controller.change_app(&show_name_app);
    break;
  case BUTTON_OK:
    // save the name and exit
    show_name_app.SetName(editor.text);
    badge_controller.change_app(&show_name_app);
    break;
  }
}

} // namespace hitcon
