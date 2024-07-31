#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <Logic/IrLogic.h>
#include <Logic/GameLogic.h>
#include <Service/IrService.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

enum class packet_type: uint8_t {
  kGame = 0,
  kShow = 1,
};

namespace hitcon {
namespace ir {

/*Definition of IR content.*/
struct GamePacket {
  uint8_t col;
  uint8_t data[hitcon::game::kDataSize];
};

struct ShowPacket {
  char message[16];
};

/*Definition of IR content.*/
struct IrData {
  uint8_t ttl;
  packet_type type;
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
  // TODO: Tune the quadratic function parameters
  uint8_t v[3] = {1, 27, 111};

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

extern IrController irController;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
