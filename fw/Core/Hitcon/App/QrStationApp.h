#ifndef HITCON_APP_QR_STATION_APP_H_
#define HITCON_APP_QR_STATION_APP_H_

#include <App/MenuApp.h>
#include <Hitcon.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_QR_STN

namespace hitcon {
namespace app {
namespace qr {

// Free function adapters for `menu_entry_t.func`
void OnUnlockPicked();
void OnWrongPicked();

constexpr menu_entry_t qr_station_menu_entries[] = {
// {"Unlock QR", nullptr, &OnUnlockPicked},
// {"Wrong", nullptr, &OnWrongPicked},
#if STATION_ID == 0
    // hitcon topic this year
    {"Agentic AI", nullptr, &OnUnlockPicked},
    {"Drone", nullptr, &OnWrongPicked},
    {"War", nullptr, &OnWrongPicked},
#elif STATION_ID == 1
    // edition of HITCON event
    {"22", nullptr, &OnUnlockPicked},
    {"24", nullptr, &OnWrongPicked},
    {"26", nullptr, &OnWrongPicked},
#elif STATION_ID == 2
    // the cat we have
    {"A", nullptr, &OnWrongPicked},
    {"B", nullptr, &OnWrongPicked},
    {"C", nullptr, &OnUnlockPicked},
#endif
};

constexpr int qr_station_menu_entries_len =
    sizeof(qr_station_menu_entries) / sizeof(menu_entry_t);

class QrStationApp : public MenuApp {
 public:
  QrStationApp()
      : MenuApp(qr_station_menu_entries, qr_station_menu_entries_len) {}
  virtual ~QrStationApp() = default;

  void Init();

  // Show "QR STN" idle text. Local buttons do nothing in any state — menu
  // navigation only comes via forwarded packets from a connected client.
  void OnEntry() override;
  void OnButton(button_t) override {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}

  // Switch between idle text and menu display when a client connects /
  // disconnects. Called by BadgeController on Peer2025 connect/disconnect.
  void OnPeerConnected();
  void OnPeerDisconnected();

  // Handler for when the client picks the right/wrong menu entry
  void OnUnlockPicked();
  void OnWrongPicked();

 private:
  // Client can only make one selection per connection
  bool selection_made_ = false;

  void OnForwardedButtonPacket(void *arg);
};

extern QrStationApp qr_station_app;

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_QR_STN

#endif  // HITCON_APP_QR_STATION_APP_H_
