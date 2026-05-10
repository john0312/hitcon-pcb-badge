#include "QrClientApp.h"

#if BADGE_ROLE != BADGE_ROLE_QR_STN

#include <Logic/Display/display.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

#include <cstdint>

using namespace hitcon::service::xboard;

namespace hitcon {

QrClientApp qr_client_app;

namespace {
struct QrStnBtnPacket {
  uint16_t button;
} __attribute__((packed));
}  // namespace

void QrClientApp::Init() {}

void QrClientApp::OnEntry() { display_set_mode_scroll_text("Controlling..."); }

void QrClientApp::OnExit() {}

void QrClientApp::OnButton(button_t button) {
  QrStnBtnPacket pkt{static_cast<uint16_t>(button)};
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt),
                                QR_STN_BTN_FWD_ID);
}

}  // namespace hitcon

#endif  // BADGE_ROLE != BADGE_ROLE_QR_STN
