#ifndef HITCON_APP_APP_H_
#define HITCON_APP_APP_H_

#include <Util/callback.h>
#include <Service/ButtonService.h>

namespace hitcon {

class App {
 public:
  App();
  virtual ~App() = default;

  // If an App is started, OnEntry will be called.
  // Once OnEntry finishes, the callback with its args will be scheduled.
  virtual void OnEntry(callback_t callback, void* callback_arg1, void* callback_arg2) = 0;

  // If an App is going to be terminated, OnExit will be called.
  // Once OnExit finishes, the callback with its args will be scheduled.
  virtual void OnExit(callback_t callback, void* callback_arg1, void* callback_arg2) = 0;

  // When an App is running, any relevant button press will result in OnButton being called.
  virtual void OnButton(button_t button) = 0;
};

}  // namespace hitcon

#endif  // #ifndef HITCON_APP_APP_H_
