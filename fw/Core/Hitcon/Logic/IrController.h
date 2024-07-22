#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <stddef.h>
#include <stdint.h>

#include <Logic/game.h>
#include <Logic/IrLogic.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {
namespace ir {

/*Definition of IR content.*/
struct IrData {
  uint8_t col;
  uint8_t data[GAME_DATA_SIZE];
};

class IrController {
 public:
  IrController();

  void Init();
  void Send2Game();
  void InitBroadcastService(uint8_t game_types);
 private:
  bool send_lock;
  bool recv_lock;
  uint8_t v[3] = {1, 1, 0};

  uint8_t callback_col;
  uint8_t *callback_data;

  hitcon::service::sched::PeriodicTask routine_task;
  hitcon::service::sched::Task task;

  // Called every 1s.
  void RoutineTask(void* unused);

  // Called on every packet.
  void OnPacketReceived(void* arg);

  int prob_f(int);

  void BroadcastIr();   
};

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
