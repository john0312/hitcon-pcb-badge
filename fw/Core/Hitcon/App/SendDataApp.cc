#include <App/SendDataApp.h>
#include <Logic/XBoardGameController.h>

using hitcon::xboard_game_controller::g_xboard_game_controller;

namespace hitcon {

SendDataApp g_send_data_app;

SendDataApp::SendDataApp() {}

void SendDataApp::Init() {}

void SendDataApp::OnEntry() {
  g_xboard_game_controller.SendPartialData(25);
  display_set_mode_scroll_text("Sending...");
}

void SendDataApp::OnExit() {
  // Nothing.
}

void SendDataApp::OnButton(button_t button) {
  // Unused.
}

}  // namespace hitcon
