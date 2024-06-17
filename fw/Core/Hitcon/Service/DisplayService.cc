#define SERVICE_DISPLAY_SERVICE_CC_

#include <Service/DisplayService.h>
#include <Service/Sched/Task.h>
#include <Service/Sched/Scheduler.h>
#include <Logic/Display/display.h>
#include <main.h>
#include <tim.h>

using namespace hitcon::service::sched;
using namespace hitcon;
namespace hitcon {

DisplayService g_display_service;

DisplayService::DisplayService() : task(15, (task_callback_t)&DisplayService::cb, (void*) this){
}

/*
 * DMA Mode: Circular
 * 1. double buffered
 * 2. first half complete
 * 3. HalfTranferComplete called
 * 4. request new, queue new Task to call request_frame_callback (get first new frame buffer)
 * 5. if the Task hasn't been executed, then run the second buffer
 */


request_cb_param tmp;
void HalfTransferComplete(DMA_HandleTypeDef* hdma) {
  if(hdma == &hdma_tim2_ch1) {
      tmp.p1 = g_display_service.request_frame_callback_arg1;
      tmp.p2 = nullptr;
      scheduler.Queue(&(g_display_service.task), &tmp);
  }
}

void DisplayService::Init() {
  tmp.p1 = request_frame_callback_arg1;
  tmp.p2 = nullptr;
  scheduler.Queue(&task, &tmp);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  hdma_tim2_ch1.XferHalfCpltCallback = &HalfTransferComplete;
  HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t) this->double_buffer, (uint32_t) &GPIOB->BSRR, DISPLAY_FRAME_SIZE*DISPLAY_FRAME_BATCH*2);
  current_buffer_index = 0;
}

void DisplayService::SetRequestFrameCallback(callback_t callback, void* callback_arg1) {

  this->request_frame_callback = callback;
  this->request_frame_callback_arg1 = callback_arg1;
  callback(callback_arg1, nullptr);
}

void DisplayService::PopulateFrames(uint8_t* buffer) {
  const uint16_t gpio_pin[8] = { 15, 14, 13, 12, 11, 10, 2, 1 };
  uint8_t test[16] = {8, 0, 9, 1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7};
  for(uint8_t k = 0; k < DISPLAY_FRAME_BATCH; k++) {
    for (uint8_t i = 0; i < DISPLAY_WIDTH; i++) {
	uint32_t temp = 0;
	for (uint8_t j = 0; j < DISPLAY_HEIGHT; j++) {
	    uint8_t index = i * DISPLAY_HEIGHT + j;
//	    if (i >= 8)
//	      index -= 8+16*8;
	    if (buffer[index] == 0)
	      temp |= (1 << 16 << gpio_pin[j]);
	    else
	      temp |= (1 << gpio_pin[j]);
	}
	for (uint8_t j = 0; j < 4; j++) {
	    if ((test[i] >> (3 - j) & 1) == 0)
	      temp |= (1 << 16 << (9 - j));
	    else
	      temp |= (1 << (9 - j));
	}

	double_buffer[i + k * DISPLAY_FRAME_SIZE + current_buffer_index * DISPLAY_FRAME_SIZE * DISPLAY_FRAME_BATCH] = temp;
    }
  }
}

//void ConvertFrameToGpioState() {
//
//}

void DisplayService::SetBrightness(uint8_t brightness) {
  uint8_t value = brightness / 10.0 * (htim3.Init.Period);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, value);
}
}  // namespace hitcon


/*
 #include <stdio.h>

int
main ()
{
  int row = 1;
  int frame[64] = {  0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1};
  int gpio_pin[8] = { 15, 14, 13, 12, 11, 10, 2, 1 };
  int data = 0;
  for(int j =0 ; j<8; j++) {
      row = j;
    for (int i = 0; i < 8; i++)
  {
    if (frame[i + row*8] == 0)
    data |= (1 << 16 << gpio_pin[i]);
    else
    data |= (1 << gpio_pin[i]);
  }
    for (int i = 0; i < 4; i++)
  {             // 9 ~ 6
    int tmp = row >> (3 - i) & 1;
    if (tmp == 0)
    data |= (1 << 16 << (9 - i));
    else
    data |= (1 << (9 - i));
  }
  data |= (1<<16<<5);
  printf ("Row %d : %X\n", row, data);
  data=0;
  }
  return 0;
}
 */
