#define SERVICE_DISPLAY_SERVICE_CC_

#include <Service/DisplayService.h>

namespace hitcon {

DisplayService g_display_service;

DisplayService::DisplayService() {
}

void DisplayService::Init() {
  // TODO
}

void DisplayService::SetRequestFrameCallback(callback_t callback, void* callback_arg1) {
  // TODO
}

void DisplayService::PopulateFrames(uint8_t* buffer) {
  // TODO
}

}  // namespace hitcon
