#ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
#define HITCON_SERVICE_BUTTON_SERVICE_H_

#include <Util/callback.h>
#include <gpio.h>

#include <cstddef>

namespace hitcon {

constexpr size_t BUTTON_AMOUNT = 8;
constexpr uint16_t btn_pins[BUTTON_AMOUNT] = {BtnA_Pin, BtnB_Pin, BtnC_Pin,
                                              BtnD_Pin, BtnE_Pin, BtnF_Pin,
                                              BtnG_Pin, BtnH_Pin};
constexpr size_t kDatasetSize = 4;

class ButtonService {
 public:
  ButtonService();

  void Init();

  // Everytime a collection of kDatasetSize of PA register is collected,
  // this callback will be called with a pointer to an uint16_t array of
  // size kDatasetSize.
  void SetDataInCallback(callback_t callback, void* callback_arg1);

  uint16_t raw_data[kDatasetSize];
  callback_t data_in_callback;
  void* data_in_callback_arg1;
};

extern ButtonService g_button_service;

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_BUTTON_SERVICE_H_
