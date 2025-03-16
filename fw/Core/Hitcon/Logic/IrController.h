#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <Logic/IrLogic.h>
#include <Service/IrService.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

enum class packet_type : uint8_t {
  kGame = 0,  // disabled
  kShow = 1,
  kTest = 2,
  // Packet types for 2025
  kAcknowledge = 3,
  kProximity = 4,
  kPubAnnounce = 5,
  kActivity = 6,
  kScoreAnnonce = 7
};

namespace hitcon {
namespace ir {

/*Definition of IR content.*/
struct GamePacket {
  // It's a placeholder after removing the ir game
  uint8_t data;
};

struct ShowPacket {
  char message[16];
};

constexpr size_t PACKET_HASH_LEN = 6;
constexpr size_t IR_USERNAME_LEN = 4;
constexpr size_t ECC_SIGNATURE_SIZE = 16;
constexpr size_t ECC_PUBKEY_SIZE = 8;

// This packet acknowledges a particular packet has been received.
struct AcknowledgePacket {
  // Hash of the packet being acknowledge.
  uint8_t packet_hash[PACKET_HASH_LEN];
};

// This packet is from the badge, saying I'm here to the base station.
struct ProximityPacket {
  uint8_t username[IR_USERNAME_LEN];
  // How much power or how active is the user according to accelerometer?
  uint8_t power;
  uint8_t nonce[2];
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the badge, announcing its public key.
struct PubAnnouncePacket {
  uint8_t pubkey[ECC_PUBKEY_SIZE];
};

// This packet is sent from two parties that participated in an activity.
struct ActivityPacket {
  uint8_t user1[IR_USERNAME_LEN];
  uint8_t user2[IR_USERNAME_LEN];
  uint8_t game_data[5];
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the base station, telling user their score.
struct ScoreAnnouncePacket {
  uint8_t user[IR_USERNAME_LEN];
  uint32_t score;
};

/*Definition of IR content.*/
struct IrData {
  uint8_t ttl;
  packet_type type;
  union {
    struct GamePacket game;
    struct ShowPacket show;
    struct ProximityPacket proximity;
    struct PubAnnouncePacket pub_announce;
    struct ActivityPacket activity;
    struct ScoreAnnouncePacket score_announce;
  };
};

class IrController {
 public:
  IrController();

  void Init();
  void ShowText(void* arg);
  void InitBroadcastService(uint8_t game_types);

  void SetDisableBroadcast() { disable_broadcast = true; }

 private:
  bool send_lock;
  bool recv_lock;
  // TODO: Tune the quadratic function parameters
  uint8_t v[3] = {1, 27, 111};
  bool disable_broadcast;

  // Number of packets received, primarily for debugging.
  size_t received_packet_cnt;

  hitcon::service::sched::PeriodicTask routine_task;
  hitcon::service::sched::Task showtext_task;
  hitcon::service::sched::Task broadcast_task;

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
