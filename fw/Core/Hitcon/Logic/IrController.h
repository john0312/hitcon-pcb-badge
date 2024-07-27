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
  uint8_t type;
  uint8_t data[GAME_DATA_SIZE];
  score_t score;
};

class IrController {
 public:
  IrController();

  void Init();

  void SendIr2Game(IrPacket &packet);
  void InitBroadcastService(uint8_t game_types);
 private:

  hitcon::service::sched::PeriodicTask routine_task;

  // Called every 1s.
  void RoutineTask(void* unused);

  // Called on every packet.
  void OnPacketReceived(void* arg);

  IrPacket BroadcastIr(IrData ir_data);   
};

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
