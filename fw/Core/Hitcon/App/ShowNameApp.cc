#include "ShowNameApp.h"
#include <Logic/Display/display.h>
#include <cstring>

namespace hitcon {

ShowNameApp::ShowNameApp() { strcpy(name, DEFAULT_NAME); }

void ShowNameApp::OnEntry() { display_set_mode_scroll_text(name); }

void ShowNameApp::OnExit() {}

void ShowNameApp::OnButton(button_t button) {}

void ShowNameApp::SetName(const char *name) { strcpy(this->name, name); }

} // namespace hitcon
