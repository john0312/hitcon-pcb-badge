#ifndef MENU_APP_H
#define MENU_APP_H

#include "app.h"

namespace hitcon {

class MenuApp : public App {
 public:
  MenuApp();
  virtual ~MenuApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern MenuApp menu_app;

}  // namespace hitcon

#endif  // MENU_APP_H
