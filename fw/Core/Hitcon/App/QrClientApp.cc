#include "QrClientApp.h"

#if BADGE_ROLE != BADGE_ROLE_QR_STN

#include <App/QrCommon.h>
#include <Logic/Display/display.h>
#include <Logic/NvStorage.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

#include <cstdint>
#include <cstring>

using namespace hitcon::service::xboard;

namespace hitcon {
namespace app {
namespace qr {

QrClientApp qr_client_app;

namespace {
constexpr uint8_t kQrUnlockStationCount = 3;
}  // namespace

void QrClientApp::Init() {
  g_xboard_logic.SetOnPacketArrive(CB_CAST(&QrClientApp::OnUnlockPacket), this,
                                   QR_STN_UNLOCK_ID);
}

void QrClientApp::OnEntry() { display_set_mode_scroll_text("Controlling..."); }

void QrClientApp::OnExit() {}

void QrClientApp::OnButton(button_t button) {
  QrStnBtnPacket pkt{static_cast<uint16_t>(button)};
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt),
                                QR_STN_BTN_FWD_ID);
}

void QrClientApp::OnUnlockPacket(void *arg) {
  if (g_xboard_logic.GetPeer() != PeerType::QRStn2026) return;
  auto *cb_arg = static_cast<PacketCallbackArg *>(arg);
  if (cb_arg->len != sizeof(QrUnlockPacket)) return;
  QrUnlockPacket pkt;
  std::memcpy(&pkt, cb_arg->data, sizeof(pkt));
  if (pkt.station_id >= kQrUnlockStationCount) return;

  nv_storage_content &nv = g_nv_storage.GetCurrentStorage();
  nv.qr_unlocked_mask |= static_cast<uint8_t>(1 << pkt.station_id);
  g_nv_storage.MarkDirty();

  display_set_mode_scroll_text("Unlocked!");
}

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // BADGE_ROLE != BADGE_ROLE_QR_STN
