#include "MainMenuApp.h"

#include <App/ShowNameApp.h>

namespace hitcon {

menu_entry_t main_menu_entries[] = {
    // TODO : change app
    {"BadUSB", &show_name_app, nullptr},
    {"Snake", &show_name_app, nullptr},
    {"Dino", &show_name_app, nullptr},
    {"Tetris", &show_name_app, nullptr},
};

MainMenuApp main_menu;

}  // namespace hitcon
