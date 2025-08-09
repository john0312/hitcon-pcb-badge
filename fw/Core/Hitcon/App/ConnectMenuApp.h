#ifndef CONNECT_MENU_APP_H
#define CONNECT_MENU_APP_H

#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/SponsorResp.h>
#include <App/TamaApp.h>
#include <App/TetrisApp.h>
#include <Hitcon.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

namespace hitcon {

using hitcon::app::snake::snake_app;
using hitcon::app::tama::tama_app;
using hitcon::app::tetris::tetris_app;
// using hitcon::app::tetris

constexpr menu_entry_t connect_menu_entries[] = {
#if BADGE_ROLE == BADGE_ROLE_SPONSOR
    {"Send Bonus", &hitcon::sponsor::g_sponsor_resp, nullptr},
#endif  // BADGE_ROLE == BADGE_ROLE_SPONSOR
    {"HackerPet", &tama_app, &hitcon::app::tama::SetMultiplayer},
    {"Tetris", &tetris_app, &hitcon::app::tetris::SetMultiplayer},
    {"Snake", &snake_app, &hitcon::app::snake::SetMultiplayer},
};

constexpr int connect_menu_entries_len =
    sizeof(connect_menu_entries) / sizeof(menu_entry_t);

class ConnectMenuApp : public MenuApp {
 public:
  ConnectMenuApp() : MenuApp(connect_menu_entries, connect_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern ConnectMenuApp connect_menu;

constexpr menu_entry_t connect_legacy_menu_entries[] = {
    {"Unsupported 2024 FW", nullptr, nullptr},
};

constexpr int connect_legacy_menu_entries_len =
    sizeof(connect_legacy_menu_entries) / sizeof(menu_entry_t);

class ConnectLegacyMenuApp : public MenuApp {
 public:
  ConnectLegacyMenuApp()
      : MenuApp(connect_legacy_menu_entries, connect_legacy_menu_entries_len) {}
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}
};

extern ConnectLegacyMenuApp connect_legacy_menu;

constexpr menu_entry_t connect_basestn_menu_entries[] = {
    {"BaseStation", nullptr, nullptr},
    {"Tama Heal", &tama_app, &hitcon::app::tama::SetBaseStationConnect},
};

constexpr int connect_basestn_menu_entries_len =
    sizeof(connect_basestn_menu_entries) / sizeof(menu_entry_t);

class ConnectBasestnMenuApp : public MenuApp {
 public:
  ConnectBasestnMenuApp()
      : MenuApp(connect_basestn_menu_entries,
                connect_basestn_menu_entries_len) {}

  void OnEntry() override;
  void OnButton(button_t button) override;

  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}

  void NotifyIrXbFinished();

 private:
  // False if IR-XB Bridge is working.
  bool basestn_available_;
};

extern ConnectBasestnMenuApp connect_basestn_menu;

}  // namespace hitcon

#endif