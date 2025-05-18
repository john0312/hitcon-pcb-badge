#include "TamaApp.h"

#include <Logic/NvStorage.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {
namespace app {
namespace tama {
TamaApp tama_app;

TamaApp::TamaApp()
    : _routine_task(930,
                    (hitcon::service::sched::task_callback_t)&TamaApp::Routine,
                    (void*)this, INTERVAL),
      _tama_data(g_nv_storage.GetCurrentStorage().tama_storage) {}

void TamaApp::Init() {
  hitcon::service::sched::scheduler.Queue(&_routine_task, nullptr);
}

void TamaApp::OnEntry() {}

void TamaApp::OnExit() {}

void TamaApp::OnButton(button_t button) {}

void TamaApp::OnEdgeButton(button_t button) {}

void TamaApp::Routine(void* unused) {}
}  // namespace tama
}  // namespace app
}  // namespace hitcon
