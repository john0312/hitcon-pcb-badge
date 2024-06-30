#include <Logic/ButtonLogic.h>
#include <Service/ButtonService.h>
#include <main.h>

namespace hitcon {
ButtonLogic g_button_logic;

ButtonLogic::ButtonLogic() {}

void ButtonLogic::Init() {
  g_button_service.SetDataInCallback((callback_t)&ButtonLogic::OnReceiveData, this);
  for(uint8_t i = 0; i < BUTTON_AMOUNT; i++)
    _count[i] = 0;
  _fire = 0;
}

void ButtonLogic::SetCallback(callback_t callback, void *callback_arg1) {
  this->callback = callback;
  this->callback_arg1 = callback_arg1;
}

void ButtonLogic::OnReceiveData(uint8_t* data) {
  static uint8_t counter = 0;
  uint16_t pressed_btn = 0, temp;
  for(uint8_t i = 0; i < kDatasetSize; i++) {
    for(uint8_t j = 0; j < BUTTON_AMOUNT; j++) {
      if(((data[i] >> (j)) & 1) == 1) {
	_count[j]++;
	pressed_btn = j;
      } else {
	if(_fire == j)
	  _fire = 0;
	if(3 < _count[j] &&_count[j] <= 80) { //handle short press
	    temp = BUTTON_MODE + j;
	    callback(callback_arg1, reinterpret_cast<void*>(static_cast<size_t>((temp))));
	} else if(80 < _count[j]) {
	    temp = BUTTON_LONG_MODE + j;
	    callback(callback_arg1, reinterpret_cast<void*>(static_cast<size_t>((temp))));
	}
	_count[j] = 0;
      }
    }
  }
  if(_fire != 0) {
    if(counter == 5) {
      counter = 0;
      temp = BUTTON_MODE + _fire;
      callback(callback_arg1, reinterpret_cast<void*>(static_cast<size_t>((temp))));
    }
    counter++;
  }

  if(_count[pressed_btn] > 300) {
    _fire = pressed_btn;
  }

}

} // namespace hitcon
