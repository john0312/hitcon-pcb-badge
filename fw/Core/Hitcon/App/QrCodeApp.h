#ifndef HITCON_APP_QR_CODE_APP_H_
#define HITCON_APP_QR_CODE_APP_H_

#include <App/app.h>

namespace hitcon {
namespace app {
namespace qr {

class QrCodeApp : public App {
 public:
  QrCodeApp() = default;
  virtual ~QrCodeApp() = default;

  void OnEntry() override;
  void OnExit() override {}
  void OnButton(button_t button) override;
  void OnEdgeButton(button_t button) override;

 private:
  int offset_x_ = 0;
  int offset_y_ = 0;
  uint8_t unlocked_mask_ = 0;
  // Tracks physical MODE key state for the MODE+OK reset combo.
  bool mode_key_down_ = false;
  void Render();
};

extern QrCodeApp qr_code_app;

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // HITCON_APP_QR_CODE_APP_H_
