#include "BadgeController.h"

#include <App/ConnectMenuApp.h>
#include <App/EditNameApp.h>
#include <App/HardwareTestApp.h>
#include <App/MainMenuApp.h>
#include <App/ShowNameApp.h>
#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>
#include <Secret/secret.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Checks.h>

using hitcon::ir::irController;
using hitcon::service::sched::my_assert;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::UsartConnectState;

namespace hitcon {
BadgeController badge_controller;

int combo_button_ctr = 0;

BadgeController::BadgeController() : current_app(nullptr) {}

void BadgeController::Init() {
  g_button_logic.SetCallback((callback_t)&BadgeController::OnButton, this);
  g_button_logic.SetEdgeCallback((callback_t)&BadgeController::OnEdgeButton,
                                 this);
  current_app = &show_name_app;
  current_app->OnEntry();
  hitcon::service::xboard::g_xboard_logic.SetOnConnectPeer2025(
      (callback_t)&BadgeController::OnXBoardConnect, this);
  hitcon::service::xboard::g_xboard_logic.SetOnDisconnectPeer2025(
      (callback_t)&BadgeController::OnXBoardDisconnect, this);

  hitcon::service::xboard::g_xboard_logic.SetOnConnectLegacy(
      (callback_t)&BadgeController::OnXBoardLegacyConnect, this);
  hitcon::service::xboard::g_xboard_logic.SetOnDisconnectLegacy(
      (callback_t)&BadgeController::OnXBoardDisconnect, this);

  hitcon::service::xboard::g_xboard_logic.SetOnConnectBaseStn2025(
      (callback_t)&BadgeController::OnXBoardBasestnConnect, this);
  hitcon::service::xboard::g_xboard_logic.SetOnDisconnectBaseStn2025(
      (callback_t)&BadgeController::OnXBoardDisconnect, this);
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

  UsartConnectState conn_state = g_xboard_logic.GetConnectState();
  if (conn_state == UsartConnectState::ConnectPeer2025) {
    change_app(&connect_menu);
  } else if (conn_state == UsartConnectState::ConnectLegacy) {
    change_app(&connect_legacy_menu);
  } else if (conn_state == UsartConnectState::ConnectBaseStn2025) {
    change_app(&connect_basestn_menu);
  } else {
    change_app(&main_menu);
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
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&connect_menu);
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
  if (current_app != &hardware_test_app)
    badge_controller.change_app(&show_name_app);
}

void BadgeController::OnEdgeButton(void *arg1) {
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg1));
  current_app->OnEdgeButton(button);
}

void BadgeController::SetStoredApp(App *app) { stored_app = app; }

void BadgeController::RestoreApp() {
  if (stored_app) change_app(stored_app);
  stored_app = nullptr;
}

}  // namespace hitcon
