#include <cstdint>
#include "BadgeController.h"

namespace hitcon
{

BadgeController::BadgeController(ButtonService &button_service) {
  button_service.SetCallback(OnButton, this);
}

void BadgeController::OnButton(void* arg1, void* arg2) {
  BadgeController* this_ = static_cast<BadgeController*>(arg1);
  button_t button = static_cast<button_t>(reinterpret_cast<uintptr_t>(arg2));

  // TODO
}

} // namespace hitcon
