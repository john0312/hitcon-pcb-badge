
#include <Logic/DisplayLogic.h>
#include <Service/DisplayService.h>
#include <Logic/Display/display.h>

namespace hitcon {
DisplayLogic g_display_logic;

DisplayLogic::DisplayLogic() {
}

void DisplayLogic::Init() {
  // TODO: Verify this.
  g_display_service.SetRequestFrameCallback((callback_t)&DisplayLogic::OnRequestFrame, this);
  frame_ = 0;
}

void DisplayLogic::OnRequestFrame(void* unused) {
  // TODO: Verify this.
  for (uint8_t i = 0; i < static_cast<int>(DISPLAY_FRAME_BATCH); i++) {
    display_get_frame(&buffer_[DISPLAY_HEIGHT*DISPLAY_WIDTH*i], frame_);
    frame_++;
  }
  g_display_service.PopulateFrames(buffer_);
}

}  // namespace hitcon
