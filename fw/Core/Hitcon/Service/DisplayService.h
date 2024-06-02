#ifndef SERVICE_DISPLAY_SERVICE_H_
#define SERVICE_DISPLAY_SERVICE_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>
#include <Service/DisplayInfo.h>

namespace hitcon {

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
  void PopulateFrames(uint8_t* buffer);

  // 0-100
  void SetBrightness(int brightness);
};

#ifndef SERVICE_DISPLAY_SERVICE_CC_
extern DisplayService g_display_service;
#endif

}  // namespace hitcon

#endif  // #ifndef SERVICE_DISPLAY_SERVICE_H_
