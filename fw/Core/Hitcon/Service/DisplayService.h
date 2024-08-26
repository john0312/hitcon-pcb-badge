#ifndef SERVICE_DISPLAY_SERVICE_H_
#define SERVICE_DISPLAY_SERVICE_H_

#include <Logic/Display/display.h>
#include <Service/DisplayInfo.h>
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

using namespace hitcon::service::sched;

namespace hitcon {
typedef struct CB_Param {
  void* callback;
  uint8_t buf_index;
} request_cb_param;

class DisplayService {
 public:
  DisplayService();

  // Init should:
  // - Setup TIM2 to run at 9.5kHz.
  // - Setup TIM2_CH1 to fire DMA trigger.
  // - Setup DMA1 CH5 to write to PB GPIO.
  void Init();

  // This callback will be called whenever DisplayService wants to pull a
  // set of frames from the upper layer.
  // The callback should call PopulateFrames().
  void SetRequestFrameCallback(callback_t callback, void* callback_arg1);

  // After RequestFrame callback is triggered, this should be called by upper
  // layer to send frame to DisplayService. Each call should contain
  // DISPLAY_FRAME_BATCH of frames.
  void PopulateFrames(display_buf_t* buffer, size_t index);

  // 0-10
  void SetBrightness(uint8_t brightness);

  void* request_frame_callback_arg1;
  Task task;

 private:
  void RequestFrameWrapper(request_cb_param* arg);

  callback_t request_frame_callback;
  uint8_t current_buffer_index;
  uint32_t double_buffer[DISPLAY_FRAME_SIZE * DISPLAY_FRAME_BATCH * 2];
};

#define DISPLAY_MAX_BRIGHTNESS 10

#ifndef SERVICE_DISPLAY_SERVICE_CC_
extern DisplayService g_display_service;
extern uint8_t g_display_brightness;
extern uint8_t g_display_standby;
#endif
}  // namespace hitcon

#endif  // #ifndef SERVICE_DISPLAY_SERVICE_H_
