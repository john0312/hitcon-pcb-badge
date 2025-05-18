#ifndef TAMA_APP_H
#define TAMA_APP_H

#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "../app.h"
#include "screens.h"

using namespace hitcon::service::sched;

namespace hitcon {
namespace app {
namespace tama {

enum TAMA_APP_STATE {
  CHOOSE_TYPE,
  EGG,
};

enum TAMA_TYPE {
  NONE,
  DOG,
  CAT,
};

typedef struct {
  TAMA_APP_STATE state;
  TAMA_TYPE type;
} tama_storage_t;

class TamaApp : public App {
 private:
  static constexpr unsigned INTERVAL = 150;
  hitcon::service::sched::PeriodicTask _routine_task;
  tama_storage_t _tama_data;
  uint32_t _current_state;
  uint32_t _next_state;
  uint32_t _start_tick;
  uint8_t _count;

 public:
  TamaApp();
  virtual ~TamaApp() = default;
  void Init();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  void OnEdgeButton(button_t button) override;
  void Routine(void* unused);
};

extern TamaApp tama_app;

}  // namespace tama
}  // namespace app
}  // namespace hitcon

#endif  // TAMA_APP_H
