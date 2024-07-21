#define SERVICE_DISPLAY_SERVICE_CC_

#include <Logic/Display/display.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>
#include <main.h>
#include <tim.h>

using namespace hitcon::service::sched;
using namespace hitcon;
namespace hitcon {
DisplayService g_display_service;

DisplayService::DisplayService()
    : task(105, (task_callback_t)&DisplayService::RequestFrameWrapper,
           (void*)this) {}

/*
 * DMA Mode: Circular
 * 1. double buffered
 * 2. first half complete
 * 3. HalfTranferComplete called
 * 4. request new, queue new Task to call request_frame_callback (get first new
 * frame buffer)
 * 5. if the Task hasn't been executed, then run the second buffer
 */

request_cb_param tmp_request_cb_param;
void DisplayTransferHalfComplete(DMA_HandleTypeDef* hdma) {
  tmp_request_cb_param.callback = g_display_service.request_frame_callback_arg1;
  tmp_request_cb_param.buf_index = 0;
  scheduler.Queue(&(g_display_service.task), &tmp_request_cb_param);
}

void DisplayTransferComplete(DMA_HandleTypeDef* hdma) {
  tmp_request_cb_param.callback = g_display_service.request_frame_callback_arg1;
  tmp_request_cb_param.buf_index = 1;
  scheduler.Queue(&(g_display_service.task), &tmp_request_cb_param);
}

void DisplayService::Init() {
  tmp_request_cb_param.callback = request_frame_callback_arg1;
  tmp_request_cb_param.buf_index = 0;
  scheduler.Queue(&task, &tmp_request_cb_param);
  HAL_TIM_PWM_Start(&htim3,
                    TIM_CHANNEL_2);  // decoder enable to control brightness
  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
  HAL_TIM_Base_Start(&htim1);  // start dma trigger dma ch5 to control matrix

  htim1.hdma[TIM_DMA_ID_UPDATE]->XferHalfCpltCallback =
      &DisplayTransferHalfComplete;
  htim1.hdma[TIM_DMA_ID_UPDATE]->XferCpltCallback = &DisplayTransferComplete;
  HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_UPDATE], (uint32_t)this->double_buffer,
                   (uint32_t)&GPIOB->BSRR,
                   DISPLAY_FRAME_SIZE * DISPLAY_FRAME_BATCH * 2);

  current_buffer_index = 0;
}

void DisplayService::SetRequestFrameCallback(callback_t callback,
                                             void* callback_arg1) {
  this->request_frame_callback = callback;
  this->request_frame_callback_arg1 = callback_arg1;
  callback(callback_arg1, nullptr);
}

void DisplayService::PopulateFrames(uint8_t* buffer) {
  const uint16_t gpio_pin[8] = {15, 14, 13, 12, 11, 10, 2, 1};
  uint8_t frame_map[16] = {8,  0, 9,  1, 10, 2, 11, 3,
                           12, 4, 13, 5, 14, 6, 15, 7};
  for (uint8_t k = 0; k < DISPLAY_FRAME_BATCH; k++) {
    for (uint8_t i = 0; i < DISPLAY_WIDTH; i++) {
      uint32_t temp = 0;
      for (uint8_t j = 0; j < DISPLAY_HEIGHT; j++) {
        uint8_t index = i * DISPLAY_HEIGHT + j;
        if (buffer[index] == 0)
          temp |= (1 << 16 << gpio_pin[j]);
        else
          temp |= (1 << gpio_pin[j]);
      }
      for (uint8_t j = 0; j < 4; j++) {
        if ((frame_map[i] >> (3 - j) & 1) == 0)
          temp |= (1 << 16 << (9 - j));
        else
          temp |= (1 << (9 - j));
      }

      double_buffer[i + k * DISPLAY_FRAME_SIZE +
                    current_buffer_index * DISPLAY_FRAME_SIZE *
                        DISPLAY_FRAME_BATCH] = temp;
    }
  }
}

void DisplayService::RequestFrameWrapper(request_cb_param* arg) {
  request_frame_callback(arg->callback, nullptr);
  current_buffer_index = arg->buf_index;
}

void DisplayService::SetBrightness(uint8_t brightness) {
  uint8_t value = brightness / 10.0 * (htim3.Init.Period);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, value);
}
}  // namespace hitcon
