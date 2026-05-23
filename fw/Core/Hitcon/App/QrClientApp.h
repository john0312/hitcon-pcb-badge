#ifndef HITCON_APP_QR_CLIENT_APP_H_
#define HITCON_APP_QR_CLIENT_APP_H_

#include <App/app.h>
#include <Hitcon.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE != BADGE_ROLE_QR_STN

namespace hitcon {
namespace app {
namespace qr {

class QrClientApp : public App {
 public:
  QrClientApp() = default;
  virtual ~QrClientApp() = default;

  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

 private:
  void OnUnlockPacket(void *arg);
};

extern QrClientApp qr_client_app;

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // BADGE_ROLE != BADGE_ROLE_QR_STN

#endif  // HITCON_APP_QR_CLIENT_APP_H_
