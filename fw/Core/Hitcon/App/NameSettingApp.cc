#include "NameSettingApp.h"

#include <App/EditNameApp.h>
#include <App/NameDisplayApp.h>

namespace hitcon {

menu_entry_t name_setting_menu_entries[] = {
    {"EditNameApp", &edit_name_app, nullptr},
    {"NameDisplayApp", &name_display_menu, nullptr},
};

NameSettingApp name_setting_menu;

}  // namespace hitcon