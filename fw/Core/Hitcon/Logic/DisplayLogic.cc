
#include <Logic/Display/display.h>
#include <Logic/DisplayLogic.h>
#include <Service/DisplayService.h>
#include <Service/Sched/Scheduler.h>
using namespace hitcon::service::sched;
namespace hitcon {
DisplayLogic g_display_logic;

DisplayLogic::DisplayLogic()
    : task(170, (task_callback_t)&DisplayLogic::HandlePopulate, (void*)this) {}

void DisplayLogic::Init() {
  // TODO: Verify this.
  g_display_service.SetRequestFrameCallback(
      (callback_t)&DisplayLogic::OnRequestFrame, this);
  frame_ = 0;
}

void DisplayLogic::OnRequestFrame(void* unused) {
  // TODO: Verify this.
  static uint8_t i = 0;
  display_get_frame_packed(&buffer_[DISPLAY_WIDTH * i], frame_);
  scheduler.Queue(&task, (void*)&i);
  frame_++;
  i++;
  if (i == static_cast<int>(DISPLAY_FRAME_BATCH)) i = 0;
}

void DisplayLogic::HandlePopulate(void* arg) {
  uint8_t* index = reinterpret_cast<uint8_t*>(arg);

  g_display_service.PopulateFrames(buffer_, *index);

  if (*index != DISPLAY_FRAME_BATCH - 1) OnRequestFrame(nullptr);
  // queue OnRequestFrame
}

}  // namespace hitcon
