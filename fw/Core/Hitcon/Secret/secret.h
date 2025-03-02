#ifndef HITCON_SECRET_SECRET_H
#define HITCON_SECRET_SECRET_H

#include <Logic/ButtonLogic.h>

namespace hitcon {
constexpr button_t COMBO_BUTTON[] = {
    BUTTON_UP,    BUTTON_UP,   BUTTON_DOWN,  BUTTON_DOWN,       BUTTON_LEFT,
    BUTTON_RIGHT, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_BRIGHTNESS, BUTTON_MODE,
};
constexpr button_t COMBO_BUTTON_DINO[] = {
    BUTTON_DOWN, BUTTON_DOWN, BUTTON_RIGHT, BUTTON_RIGHT, BUTTON_OK};
constexpr size_t COMBO_BUTTON_LEN = sizeof(COMBO_BUTTON) / sizeof(button_t);
constexpr size_t COMBO_BUTTON_DINO_LEN =
    sizeof(COMBO_BUTTON_DINO) / sizeof(button_t);
extern int combo_button_ctr;

}  // namespace hitcon
#endif  // HITCON_SECRET_SECRET_H
