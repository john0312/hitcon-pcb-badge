#include "QrStationApp.h"

#if BADGE_ROLE == BADGE_ROLE_QR_STN

#include <App/QrCommon.h>
#include <Logic/Display/display.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

#include <cstdint>
#include <cstring>

using namespace hitcon::service::xboard;

namespace hitcon {
namespace app {
namespace qr {

QrStationApp qr_station_app;

void OnUnlockPicked() { qr_station_app.OnUnlockPicked(); }
void OnWrongPicked() { qr_station_app.OnWrongPicked(); }

void QrStationApp::Init() {
  g_xboard_logic.SetOnPacketArrive(
      CB_CAST(&QrStationApp::OnForwardedButtonPacket), this, QR_STN_BTN_FWD_ID);
}

void QrStationApp::OnEntry() {
  active = false;
  display_set_mode_scroll_text("QR STN");
}

void QrStationApp::OnPeerConnected() {
  selection_made_ = false;
  menu_entry_index = 0;
  MenuApp::OnEntry();
}

void QrStationApp::OnPeerDisconnected() {
  MenuApp::OnExit();
  display_set_mode_scroll_text("QR STN");
}

void QrStationApp::OnUnlockPicked() {
  selection_made_ = true;
  QrUnlockPacket pkt{static_cast<uint8_t>(STATION_ID)};
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt),
                                QR_STN_UNLOCK_ID);
  display_set_mode_scroll_text("YES!");
}

void QrStationApp::OnWrongPicked() {
  selection_made_ = true;
  display_set_mode_scroll_text("NO!");
}

void QrStationApp::OnForwardedButtonPacket(void *arg) {
  if (!active) return;
  if (selection_made_) return;
  auto *cb_arg = static_cast<PacketCallbackArg *>(arg);
  if (cb_arg->len != sizeof(QrStnBtnPacket)) return;
  QrStnBtnPacket pkt;
  std::memcpy(&pkt, cb_arg->data, sizeof(pkt));
  MenuApp::OnButton(static_cast<button_t>(pkt.button));
}

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_QR_STN
