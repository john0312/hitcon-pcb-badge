#ifndef TAMA_APP_H
#define TAMA_APP_H

#include <Logic/Display/display.h>
#include <Service/Sched/PeriodicTask.h>

#include "app.h"

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
  tama_storage_t& _tama_data;

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

constexpr display_buf_t tama_dog_bitmap[8] = {
    0b11100111,
    0b00111100,
    0b01000010,
    0b10000001,
    0b10100101,
    0b10000001,
    0b01000010,
    0b00111100,
};

constexpr display_buf_t tama_cat_bitmap[8] = {
    0b01000010,
    0b10100101,
    0b10011001,
    0b10000001,
    0b10100101,
    0b10000001,
    0b01000010,
    0b00111100,
};

#endif  // TAMA_APP_H
