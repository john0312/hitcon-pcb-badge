#include "EditNameApp.h"
#include <Service/ButtonService.h>

namespace hitcon {

EditNameApp::EditNameApp() {}

void EditNameApp::OnEntry() {
  editor = TextEditorDisplay("HITCON"); // TODO: where to get the initial text?
  // TODO: register editor.draw to display_service
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
    // TODO: exit without saving
    break;
  case BUTTON_OK:
    // TODO: save the name and exit (use editor.text to get the name)
    break;
  }
}

} // namespace hitcon
