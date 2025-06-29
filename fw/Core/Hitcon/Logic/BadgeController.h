#ifndef BADGE_CONTROLLER_H
#define BADGE_CONTROLLER_H

#include <App/app.h>
#include <Logic/ButtonLogic.h>

namespace hitcon {

class BadgeController {
 public:
  App *current_app;

  BadgeController();

  void Init();

  void change_app(App *new_app);

  App *GetCurrentApp() { return current_app; };

  // This is called whenever a button is pressed, usually called by the button
  // service. arg1 is this pointer, arg2 is the button_t.
  void OnButton(void *arg1);

  void OnEdgeButton(void *arg1);

  // App should call this to denote that the app has ended and wishes to return
  // to the main menu (or similar).
  void BackToMenu(App *ending_app);

  // TODO: This is called whenever a separate board connects.
  void OnXBoardConnect(void *unused);

  // TODO: This is called whenever a separate board disconnects.
  void OnXBoardDisconnect(void *unused);

  // This is called whenever a legacy peer board connects.
  void OnXBoardLegacyConnect(void *unused);

  // This is called whenever a base station board connects.
  void OnXBoardBasestnConnect(void *unused);

  // TODO: This is called by UsbService whenever we're plugged into a computer.
  // void OnUsbPlugIn();

  // for surprice use
  void SetCallback(callback_t callback, void *callback_arg1,
                   void *callback_arg2);

  // for interrupt resume
  void SetStoredApp(App *app);
  void RestoreApp();

 private:
  callback_t callback = nullptr;
  void *callback_arg1;
  void *callback_arg2;
  App *stored_app;
};

extern BadgeController badge_controller;
extern int combo_button_ctr;

}  // namespace hitcon

#endif  // BADGE_CONTROLLER_H
