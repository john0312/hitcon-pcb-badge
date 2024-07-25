#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <Logic/IrLogic.h>
#include <Logic/game.h>
#include <Service/IrService.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {
namespace ir {

/*Definition of IR content.*/
struct GamePacket {
  uint8_t col;
  uint8_t data[GAME_DATA_SIZE];
};

struct ShowPacket {
  char message[16];
};

/*Definition of IR content.*/
struct IrData {
  uint8_t ttl;
  uint8_t packet_type;
  union {
    struct GamePacket game;
    struct ShowPacket show;
  };
};

class IrController {
 public:
  IrController();

  void Init();
  void Send2Game(void* game);
  void InitBroadcastService(uint8_t game_types);

 private:
  bool send_lock;
  bool recv_lock;
  // TODO: Calculate the v[] in Init
  uint8_t v[3] = {1, 1, 0};

  hitcon::service::sched::PeriodicTask routine_task;
  hitcon::service::sched::Task send2game_task;
  hitcon::service::sched::Task broadcast_task;

  // Called every 1s.
  void RoutineTask(void* unused);

  // Called on every packet.
  void OnPacketReceived(void* arg);

  int prob_f(int);

  void BroadcastIr(void* unused);
};

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
