#include "QrStationApp.h"

#if BADGE_ROLE == BADGE_ROLE_QR_STN

#include <Logic/Display/display.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

#include <cstdint>
#include <cstring>

using namespace hitcon::service::xboard;

namespace hitcon {

QrStationApp qr_station_app;

namespace {
struct QrStnBtnPacket {
  uint16_t button;
} __attribute__((packed));
}  // namespace

void QrStationApp::Init() {
  g_xboard_logic.SetOnPacketArrive(
      CB_CAST(&QrStationApp::OnForwardedButtonPacket), this, QR_STN_BTN_FWD_ID);
}

void QrStationApp::OnEntry() {
  active = false;
  display_set_mode_scroll_text("QR STN");
}

void QrStationApp::OnPeerConnected() { MenuApp::OnEntry(); }

void QrStationApp::OnPeerDisconnected() {
  MenuApp::OnExit();
  display_set_mode_scroll_text("QR STN");
}

void QrStationApp::OnForwardedButtonPacket(void *arg) {
  if (!active) return;
  auto *cb_arg = static_cast<PacketCallbackArg *>(arg);
  if (cb_arg->len != sizeof(QrStnBtnPacket)) return;
  QrStnBtnPacket pkt;
  std::memcpy(&pkt, cb_arg->data, sizeof(pkt));
  MenuApp::OnButton(static_cast<button_t>(pkt.button));
}

}  // namespace hitcon

#endif  // BADGE_ROLE == BADGE_ROLE_QR_STN
