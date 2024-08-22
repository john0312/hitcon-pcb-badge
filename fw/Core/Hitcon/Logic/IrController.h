#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <Logic/GameLogic.h>
#include <Logic/IrLogic.h>
#include <Secret/secret.h>
#include <Service/IrService.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

enum class packet_type : uint8_t {
  kGame = 0,
  kShow = 1,
  kTest = 2,
  kPartition = hitcon::kPartitionPacketId,
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

struct PartitionPacket {
  uint8_t partition;
};

/*Definition of IR content.*/
struct IrData {
  uint8_t ttl;
  packet_type type;
  union {
    struct GamePacket game;
    struct ShowPacket show;
    struct PartitionPacket partition;
  };
};

class IrController {
 public:
  IrController();

  void Init();
  void Send2Game(void* arg);
  void ShowText(void* arg);
  void InitBroadcastService(uint8_t game_types);

  void SetDisableBroadcast() { disable_broadcast = true; }
  void SendPartitionPacket(unsigned char id);

 private:
  bool send_lock;
  bool recv_lock;
  // TODO: Tune the quadratic function parameters
  uint8_t v[3] = {1, 27, 111};
  bool disable_broadcast;

  // Number of packets received, primarily for debugging.
  size_t received_packet_cnt;

  hitcon::service::sched::PeriodicTask routine_task;
  hitcon::service::sched::Task send2game_task;
  hitcon::service::sched::Task showtext_task;
  hitcon::service::sched::Task broadcast_task;

  IrData ir_parition_data_;
  IrData priority_data_;
  size_t priority_data_len_;

  // Called every 1s.
  void RoutineTask(void* unused);

  // Called on every packet.
  void OnPacketReceived(void* arg);

  int prob_f(int);

  void BroadcastIr(void* unused);
  void SendShowPacket(char* msg);

  bool TrySendPriority();
};

extern IrController irController;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
