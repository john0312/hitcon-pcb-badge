#include "BadgeController.h"

#include <App/ConnectMenuApp.h>
#include <App/EditNameApp.h>
#include <App/HardwareTestApp.h>
#include <App/MainMenuApp.h>
#include <App/QrClientApp.h>
#include <App/QrStationApp.h>
#include <App/ShowNameApp.h>
#include <App/UsbMenuApp.h>
#include <Hitcon.h>
#include <Logic/UsbLogic.h>
#include <Logic/XBoardLogic.h>
#include <Secret/secret.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Checks.h>
#include <Service/UsbService.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_QR_STN
using hitcon::app::qr::qr_station_app;
#else
using hitcon::app::qr::qr_client_app;
#endif
using hitcon::service::sched::my_assert;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::PeerType;

namespace hitcon {
BadgeController badge_controller;

int combo_button_ctr = 0;

BadgeController::BadgeController() : current_app(nullptr) {}

void BadgeController::Init() {
  g_button_logic.SetCallback(CB_CAST(&BadgeController::OnButton), this);
  g_button_logic.SetEdgeCallback(CB_CAST(&BadgeController::OnEdgeButton), this);
#if BADGE_ROLE == BADGE_ROLE_QR_STN
  current_app = &qr_station_app;
#else
  current_app = &show_name_app;
#endif
  current_app->OnEntry();
#if BADGE_ROLE == BADGE_ROLE_QR_STN
  g_xboard_logic.SetOnConnect(
      PeerType::Peer2025, CB_CAST(&BadgeController::OnPeerControllerConnect),
      this);
  g_xboard_logic.SetOnDisconnect(
      PeerType::Peer2025, CB_CAST(&BadgeController::OnPeerControllerDisconnect),
      this);
#else
  g_xboard_logic.SetOnConnect(PeerType::Peer2025,
                              CB_CAST(&BadgeController::OnXBoardConnect), this);
  g_xboard_logic.SetOnDisconnect(
      PeerType::Peer2025, CB_CAST(&BadgeController::OnXBoardDisconnect), this);

  g_xboard_logic.SetOnConnect(
      PeerType::Legacy, CB_CAST(&BadgeController::OnXBoardLegacyConnect), this);
  g_xboard_logic.SetOnDisconnect(
      PeerType::Legacy, CB_CAST(&BadgeController::OnXBoardDisconnect), this);

  g_xboard_logic.SetOnConnect(PeerType::BaseStn2025,
                              CB_CAST(&BadgeController::OnXBoardBasestnConnect),
                              this);
  g_xboard_logic.SetOnDisconnect(PeerType::BaseStn2025,
                                 CB_CAST(&BadgeController::OnXBoardDisconnect),
                                 this);

  g_xboard_logic.SetOnConnect(PeerType::QRStn2026,
                              CB_CAST(&BadgeController::OnXBoardQrStnConnect),
                              this);
  g_xboard_logic.SetOnDisconnect(
      PeerType::QRStn2026, CB_CAST(&BadgeController::OnXBoardQrStnDisconnect),
      this);
#endif

  usb::g_usb_service.SetOnUsbPlugIn(CB_CAST(&BadgeController::OnUsbPlugIn),
                                    this);
  usb::g_usb_service.SetOnUsbPlugOut(CB_CAST(&BadgeController::OnUsbPlugOut),
                                     this);
}

void BadgeController::SetCallback(callback_t callback, void *callback_arg1,
                                  void *callback_arg2) {
  this->callback = callback;
  this->callback_arg1 = callback_arg1;
  this->callback_arg2 = callback_arg2;
}

void BadgeController::change_app(App *new_app) {
  if (current_app) current_app->OnExit();
  current_app = new_app;
  if (current_app) current_app->OnEntry();
}

void BadgeController::BackToMenu(App *ending_app) {
  my_assert(current_app == ending_app);

  switch (g_xboard_logic.GetPeer()) {
    case PeerType::Peer2025:
      change_app(&connect_menu);
      break;
    case PeerType::Legacy:
      change_app(&connect_legacy_menu);
      break;
    case PeerType::BaseStn2025:
      change_app(&connect_basestn_menu);
      break;
    default:
      if (usb::g_usb_service.IsConnected()) {
        change_app(&usb::usb_menu);
      } else {
        change_app(&main_menu);
      }
      break;
  }
}

void BadgeController::OnButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));

  if (button == COMBO_BUTTON[combo_button_ctr]) {
    combo_button_ctr++;
  } else {
    combo_button_ctr = (button == COMBO_BUTTON[0]) ? 1 : 0;
  }
  if (combo_button_ctr == COMBO_BUTTON_LEN) {
    // surprise
    combo_button_ctr = (button == COMBO_BUTTON[0]) ? 1 : 0;
    if (this->callback) {
      badge_controller.SetStoredApp(badge_controller.GetCurrentApp());
      this->callback(callback_arg1, callback_arg2);
    }
    return;
  }

  if (button == BUTTON_BRIGHTNESS) {
    g_display_brightness = g_display_brightness + 1;
    if (g_display_brightness == DISPLAY_MAX_BRIGHTNESS) {
      g_display_brightness = 1;
    }
    if (!g_display_standby) {
      g_display_service.SetBrightness(g_display_brightness);
    }
  } else if (button == BUTTON_LONG_BRIGHTNESS) {
    // stand_by mode
    g_display_standby = 1 - g_display_standby;
    if (g_display_standby) {
      g_display_service.SetBrightness(0);
    } else {
      g_display_service.SetBrightness(g_display_brightness);
    }
  } else if (g_display_standby == 1) {
    // standby wakeup
    g_display_standby = 0;
    g_display_service.SetBrightness(g_display_brightness);
  }

  // forward the button to the current app
  current_app->OnButton(button);
}

void BadgeController::OnXBoardConnect(void *unused) {
  if (current_app != &hardware_test_app) {
    badge_controller.change_app(&connect_menu);
  }
}

void BadgeController::OnXBoardLegacyConnect(void *unused) {
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&connect_legacy_menu);
}

void BadgeController::OnXBoardBasestnConnect(void *unused) {
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&connect_basestn_menu);
}

void BadgeController::OnXBoardDisconnect(void *unused) {
  if (current_app != &hardware_test_app) {
    badge_controller.change_app(&show_name_app);
  }
}

#if BADGE_ROLE == BADGE_ROLE_QR_STN
void BadgeController::OnPeerControllerConnect(void *unused) {
  if (current_app != &hardware_test_app) {
    if (current_app != &qr_station_app) {
      badge_controller.change_app(&qr_station_app);
    }
    qr_station_app.OnPeerConnected();
  }
}

void BadgeController::OnPeerControllerDisconnect(void *unused) {
  if (current_app == &qr_station_app) {
    qr_station_app.OnPeerDisconnected();
  }
}
#else
void BadgeController::OnXBoardQrStnConnect(void *unused) {
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&qr_client_app);
}

void BadgeController::OnXBoardQrStnDisconnect(void *unused) {
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&show_name_app);
}
#endif

void BadgeController::OnEdgeButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));
  current_app->OnEdgeButton(button);
}

void BadgeController::SetStoredApp(App *app) { stored_app = app; }

void BadgeController::RestoreApp() {
  if (stored_app) change_app(stored_app);
  stored_app = nullptr;
}

void BadgeController::OnUsbPlugIn(void *unused) {
  badge_controller.change_app(&usb::usb_menu);
}

void BadgeController::OnUsbPlugOut(void *unused) {
  if (GetCurrentApp() == &usb::usb_menu ||
      GetCurrentApp() == &usb::bad_usb_app || GetCurrentApp() == &show_id_app) {
    change_app(&show_name_app);
  }
}
}  // namespace hitcon
