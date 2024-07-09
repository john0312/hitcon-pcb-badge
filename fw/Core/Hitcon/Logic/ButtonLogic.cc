#include <Logic/ButtonLogic.h>
#include <Service/ButtonService.h>
#include <main.h>

namespace hitcon {
ButtonLogic g_button_logic;

ButtonLogic::ButtonLogic() : _callback_task(642, (callback_t)&ButtonLogic::CallbackWrapper, this) {}

void ButtonLogic::Init() {
  g_button_service.SetDataInCallback((callback_t)&ButtonLogic::OnReceiveData,
                                     this);
  for (uint8_t i = 0; i < BUTTON_AMOUNT; i++) _count[i] = 0;
  _fire = 0;
}

void ButtonLogic::SetCallback(callback_t callback, void* callback_arg1) {
  this->callback = callback;
  this->callback_arg1 = callback_arg1;
}

void ButtonLogic::CallbackWrapper(void* arg2) {
  if(callback)
    callback(callback_arg1, arg2);
}

void ButtonLogic::OnReceiveData(uint8_t* data) {
  static uint8_t counter = 0;
  uint16_t pressed_btn = 0;
  for (uint8_t i = 0; i < kDatasetSize; i++) {
    for (uint8_t j = 0; j < BUTTON_AMOUNT; j++) {
      if (((data[i] >> (j)) & 1) == 1) {
        _count[j]++;
        pressed_btn = j;
      } else {
        if (_fire == j) _fire = 0;
        if (3 < _count[j] && _count[j] <= 80) {  // handle short press
          _out = BUTTON_MODE + j;
          CallbackWrapper(reinterpret_cast<void*>(static_cast<size_t>((_out))));
        } else if (80 < _count[j]) {
	  _out = BUTTON_LONG_MODE + j;
	  CallbackWrapper(reinterpret_cast<void*>(static_cast<size_t>((_out))));
        }
        _count[j] = 0;
      }
    }
  }
  if (_fire != 0) {
    if (counter == 5) {
      counter = 0;
      _out = BUTTON_MODE + _fire;
      CallbackWrapper(reinterpret_cast<void*>(static_cast<size_t>((_out))));
    }
    counter++;
  }

  if (_count[pressed_btn] > 150) {
    _fire = pressed_btn;
  }
}

}  // namespace hitcon
