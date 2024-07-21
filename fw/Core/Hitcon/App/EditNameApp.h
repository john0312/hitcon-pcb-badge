#ifndef EDIT_NAME_APP_H
#define EDIT_NAME_APP_H

#include <Logic/Display/editor.h>

#include "app.h"

namespace hitcon {

/**
 * This app is used edit the name of the badge.
 *
 * - BUTTON_UP and BUTTON_DOWN will change the character at the cursor.
 * - BUTTON_LEFT and BUTTON_RIGHT will move the cursor.
 * - BUTTON_BACK will exit without saving.
 * - BUTTON_OK will save the name and exit.
 */
class EditNameApp : public App {
 private:
  TextEditorDisplay editor;

 public:
  EditNameApp();
  virtual ~EditNameApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern EditNameApp edit_name_app;

}  // namespace hitcon

#endif  // EDIT_NAME_APP_H
