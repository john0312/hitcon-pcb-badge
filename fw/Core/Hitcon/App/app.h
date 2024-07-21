#ifndef HITCON_APP_APP_H_
#define HITCON_APP_APP_H_

#include <Logic/ButtonLogic.h>
#include <Util/callback.h>

namespace hitcon {

class App {
 public:
  App() = default;
  virtual ~App() = default;

  // Once the app is activated, OnEntry will be called.
  // Display mode should be set in OnEntry.
  virtual void OnEntry() = 0;

  // If an App is going to be terminated, OnExit will be called.
  virtual void OnExit() = 0;

  // When an App is running, events should be forwarded to OnButton.
  virtual void OnButton(button_t button) = 0;
};

}  // namespace hitcon

#endif  // #ifndef HITCON_APP_APP_H_
