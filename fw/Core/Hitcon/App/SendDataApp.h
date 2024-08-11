#ifndef SEND_DATA_APP_H_
#define SEND_DATA_APP_H_

#include <App/app.h>
#include <Logic/Display/display.h>
#include <Logic/XBoardGameController.h>

namespace hitcon {

class SendDataApp : public App {
 public:
  SendDataApp();
  virtual ~SendDataApp() = default;

  void Init();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
};

extern SendDataApp g_send_data_app;

}  // namespace hitcon

#endif  // SEND_DATA_APP_H_
