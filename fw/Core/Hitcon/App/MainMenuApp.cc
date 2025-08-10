#include "MainMenuApp.h"

namespace hitcon {

MainMenuApp main_menu;

constexpr uint32_t kDebugMenuSequence = 0x189;
// RLL RLLL RRL
constexpr size_t kDebugMenuSequenceLength = 10;
void MainMenuApp::OnButton(button_t button) {
  if (button == BUTTON_LEFT || button == BUTTON_RIGHT) {
    uint8_t curr_value = 0;
    if (button == BUTTON_RIGHT) {
      curr_value = 1;
    }
    if (curr_value != ((kDebugMenuSequence >> dbg_ctr) & 1)) {
      dbg_ctr = 0;
    } else {
      dbg_ctr++;
      if (dbg_ctr >= kDebugMenuSequenceLength) {
        dbg_ctr = 0;
        badge_controller.change_app(&g_debug_app);
      }
    }
  } else {
    dbg_ctr = 0;
    MenuApp::OnButton(button);
  }
}

}  // namespace hitcon
