
#include <Logic/DisplayLogic.h>
#include <Service/DisplayService.h>
#include <Service/Display/display.h>

namespace hitcon {

void DisplayLogic::Init() {
  // TODO: Verify this.
  g_display_service.SetRequestFrameCallback((callback_t)&DisplayLogic::OnRequestFrame, this);
}

void DisplayLogic::OnRequestFrame(void *unused) {
  // TODO: Verify this.
  for (int i = 0; i < static_cast<int>(DISPLAY_FRAME_BATCH); i++) {
    display_get_frame(&buffer_[DISPLAY_FRAME_SIZE*i], frame_);
    frame_++;
  }
  g_display_service.PopulateFrames(buffer_);
}

}  // namespace hitcon
