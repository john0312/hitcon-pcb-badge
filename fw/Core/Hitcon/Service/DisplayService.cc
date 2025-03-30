#define SERVICE_DISPLAY_SERVICE_CC_

#include <Logic/Display/display.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>
#include <Service/Suspender.h>
#include <main.h>
#include <tim.h>

using namespace hitcon::service::sched;
using namespace hitcon;
namespace hitcon {
DisplayService g_display_service;
uint8_t g_display_brightness = 3;
uint8_t g_display_standby = 0;

DisplayService::DisplayService()
    : task(169, (task_callback_t)&DisplayService::RequestFrameWrapper,
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
  if (!g_suspender.IsSuspended()) {
    tmp_request_cb_param.callback =
        g_display_service.request_frame_callback_arg1;
    tmp_request_cb_param.buf_index = 0;
    scheduler.Queue(&(g_display_service.task), &tmp_request_cb_param);
  }
}

void DisplayTransferComplete(DMA_HandleTypeDef* hdma) {
  if (!g_suspender.IsSuspended()) {
    tmp_request_cb_param.callback =
        g_display_service.request_frame_callback_arg1;
    tmp_request_cb_param.buf_index = 1;
    scheduler.Queue(&(g_display_service.task), &tmp_request_cb_param);
  }
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

/* LED Matrix Layout
 *    a b c d e f g  a b c d e f g (const uint16_t gpio_pin[8])
 * 8               0
 * 9               1
 * 10              2
 * 11              3
 * 12              4
 * 13              5
 * 14              6
 * 15              7
 */
void DisplayService::PopulateFrames(display_buf_t* buffer,
                                    size_t buffer_index) {
  constexpr uint16_t gpio_pin[8] = {15, 14, 13, 12, 11, 10, 2, 1};
  // row_map[n] => set A3~A0 BSRR register
  constexpr uint32_t row_map[16] = {
      0B0000'0001'0001'1000 << 16 | 0B0000'0010'0000'0000,  // 1000
      0B0000'0011'0001'1000 << 16 | 0B0000'0000'0000'0000,  // 0000
      0B0000'0001'0001'0000 << 16 | 0B0000'0010'0000'1000,  // 1001
      0B0000'0011'0001'0000 << 16 | 0B0000'0000'0000'1000,  // 0001
      0B0000'0001'0000'1000 << 16 | 0B0000'0010'0001'0000,  // 1010
      0B0000'0011'0000'1000 << 16 | 0B0000'0000'0001'0000,  // 0010
      0B0000'0001'0000'0000 << 16 | 0B0000'0010'0001'1000,  // 1011
      0B0000'0011'0000'0000 << 16 | 0B0000'0000'0001'1000,  // 0011
      0B0000'0000'0001'1000 << 16 | 0B0000'0011'0000'0000,  // 1100
      0B0000'0010'0001'1000 << 16 | 0B0000'0001'0000'0000,  // 0100
      0B0000'0000'0001'0000 << 16 | 0B0000'0011'0000'1000,  // 1101
      0B0000'0010'0001'0000 << 16 | 0B0000'0001'0000'1000,  // 0101
      0B0000'0000'0000'1000 << 16 | 0B0000'0011'0001'0000,  // 1110
      0B0000'0010'0000'1000 << 16 | 0B0000'0001'0001'0000,  // 0110
      0B0000'0000'0000'0000 << 16 | 0B0000'0011'0001'1000,  // 1111
      0B0000'0010'0000'0000 << 16 | 0B0000'0001'0001'1000,  // 0111
  };

  if (display_set_mode_orientation) {
    for (uint8_t i = 0; i < 8; i++) {
      for (int8_t j = 1; j >= 0; j--) {  // j=0 left matrix, j=1 right
        uint32_t temp = 0;
        uint8_t current_row = 2 * i + j;
        for (uint8_t k = 0; k < 8; k++) {  // set A~G pin
          if (buffer[k + j * 8] & (1 << i))
            temp |= (1 << gpio_pin[k]);
          else
            temp |= (1 << 16 << gpio_pin[k]);
        }
        temp |= row_map[current_row];
        double_buffer[current_row + buffer_index * DISPLAY_FRAME_SIZE +
                      current_buffer_index * DISPLAY_FRAME_SIZE *
                          DISPLAY_FRAME_BATCH] = temp;
      }
    }
  } else {
    for (uint8_t i = 0; i < 8; i++) {
      for (int8_t j = 1; j >= 0; j--) {  // j=0 left matrix, j=1 right
        uint32_t temp = 0;
        uint8_t current_row = 2 * i + j;
        for (uint8_t k = 0; k < 8; k++) {  // set A~G pin
          if (buffer[(7 - k) + (1 - j) * 8] & (1 << (7 - i)))
            temp |= (1 << gpio_pin[k]);
          else
            temp |= (1 << 16 << gpio_pin[k]);
        }
        temp |= row_map[current_row];
        double_buffer[current_row + buffer_index * DISPLAY_FRAME_SIZE +
                      current_buffer_index * DISPLAY_FRAME_SIZE *
                          DISPLAY_FRAME_BATCH] = temp;
      }
    }
  }
}

void DisplayService::RequestFrameWrapper(request_cb_param* arg) {
  request_frame_callback(arg->callback, nullptr);
  current_buffer_index = arg->buf_index;
}

void DisplayService::SetBrightness(uint8_t brightness) {
  uint8_t value =
      brightness * 1.0 / DISPLAY_MAX_BRIGHTNESS * (htim3.Init.Period);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, value);
}
}  // namespace hitcon
