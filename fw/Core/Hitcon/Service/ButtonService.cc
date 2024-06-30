#define SERVICE_BUTTON_SERVICE_CC_

#include <Service/ButtonService.h>
#include <main.h>
#include <tim.h>

namespace hitcon {


ButtonService g_button_service;
ButtonService::ButtonService() {}

void TransferComplete(DMA_HandleTypeDef* hdma) {
  uint8_t output[kDatasetSize];

  for(uint8_t i = 0; i < kDatasetSize; i++) {
    uint8_t temp = 0;
    for(uint8_t j = 0; j < BUTTON_AMOUNT; j++) {
      if ((g_button_service.raw_data[i] & (btn_pins[j])) == 0) {
	temp |= 1<<j;
      }
    }
    output[i] = temp;
  }
  g_button_service.data_in_callback(g_button_service.data_in_callback_arg1, output);
    // transfer non order raw gpio data to 0~7
    // after callback finish, start new dma request

  HAL_DMA_Start_IT(&hdma_tim4_ch2, (uint32_t) &GPIOA->IDR, (uint32_t) g_button_service.raw_data, kDatasetSize);
}



void ButtonService::Init() {
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_CC2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  hdma_tim4_ch2.XferCpltCallback = &TransferComplete;
  HAL_DMA_Start_IT(&hdma_tim4_ch2, (uint32_t) &GPIOA->IDR, (uint32_t) raw_data, kDatasetSize);

//  HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t) this->double_buffer, (uint32_t) &GPIOB->BSRR, DISPLAY_FRAME_SIZE*DISPLAY_FRAME_BATCH*2);
//  current_buffer_index = 0;
}

void ButtonService::SetDataInCallback(callback_t callback, void *callback_arg1) {
  data_in_callback = callback;
  data_in_callback_arg1 = callback_arg1;
}

} // namespace hitcon
