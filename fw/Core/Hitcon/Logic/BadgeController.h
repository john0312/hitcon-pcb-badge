#ifndef BADGE_CONTROLLER_H
#define BADGE_CONTROLLER_H

#include <App/app.h>
#include <Service/ButtonService.h>

namespace hitcon {

class BadgeController {
public:
  App *current_app;

  BadgeController();

  void change_app(App *new_app);

  // This is called whenever a button is pressed, usually called by the button
  // service. arg1 is this pointer, arg2 is the button_t.
  static void OnButton(void *arg1, void *arg2);

  // TODO: This is called whenever a packet is received from cross board
  // connector, usually by CrossBoardService. void
  // OnCrossBoardPacket(XBoardPacket* packet);

  // TODO: This is called by UsbService whenever we're plugged into a computer.
  // void OnUsbPlugIn();
};

extern BadgeController badge_controller;

} // namespace hitcon

#endif // BADGE_CONTROLLER_H
