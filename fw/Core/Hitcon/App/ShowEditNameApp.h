#ifndef SHOW_EDIT_NAME_APP_H
#define SHOW_EDIT_NAME_APP_H

#include "app.h"

namespace hitcon {

class ShowEditNameApp : public App {
public:
  ShowEditNameApp();
  virtual ~ShowEditNameApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern ShowEditNameApp show_edit_name_app;

} // namespace hitcon

#endif // SHOW_EDIT_NAME_APP_H
