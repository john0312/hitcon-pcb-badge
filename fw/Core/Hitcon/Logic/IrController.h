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
  kTwoBadgeActivity = 6,
  kScoreAnnonce = 7,
  kSingleBadgeActivity = 8,
  kSponsorActivity = 9
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
constexpr size_t ECC_SIGNATURE_SIZE = 14;
constexpr size_t ECC_PUBKEY_SIZE = 8;

// This packet acknowledges a particular packet has been received.
struct AcknowledgePacket {
  // Hash of the packet being acknowledge.
  uint8_t packet_hash[PACKET_HASH_LEN];
};

// This packet is from the badge, saying I'm here to the base station.
struct ProximityPacket {
  uint8_t user[IR_USERNAME_LEN];
  // How much power or how active is the user according to accelerometer?
  uint8_t power;
  uint8_t nonce[2];
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the badge, announcing its public key.
struct PubAnnouncePacket {
  uint8_t pubkey[ECC_PUBKEY_SIZE];
  // Signature from the Certificate Authority.
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is sent from two parties that participated in an activity.
struct TwoBadgeActivityPacket {
  uint8_t user1[IR_USERNAME_LEN];
  uint8_t user2[IR_USERNAME_LEN];
  uint8_t game_data[5];
  // game_data structure:
  // Bit [0:4] - Game Type
  //          0x00 - None/Reserved
  //          0x01 - Snake
  //          0x02 - Tetris
  // Bit [4:14] - Player 1 Score
  // Bit [14:24] - Player 2 Score
  // Bit [24:40] - Nonce
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the base station, telling user their score.
struct ScoreAnnouncePacket {
  uint8_t user[IR_USERNAME_LEN];
  uint32_t score;
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the badge to the base station.
struct SingleBadgeActivityPacket {
  uint8_t user[IR_USERNAME_LEN];
  uint8_t event_type;
  // 0x01 - Snake
  // 0x02 - Tetris
  // 0x03 - Dino
  // 0x10 - Shake
  uint8_t event_data[3];
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

// This packet is from the badge to the base station.
struct SponsorActivityPacket {
  uint8_t user[IR_USERNAME_LEN];
  uint8_t sponsor_id;
  uint8_t sponsor_data[9];
};

/*Definition of IR content.*/
struct IrData {
  uint8_t ttl;
  packet_type type;
  union {
    struct GamePacket game;
    struct ShowPacket show;
    struct AcknowledgePacket acknowledge;
    struct ProximityPacket proximity;
    struct PubAnnouncePacket pub_announce;
    struct TwoBadgeActivityPacket two_activity;
    struct ScoreAnnouncePacket score_announce;
    struct SingleBadgeActivityPacket single_activity;
    struct SponsorActivityPacket sponsor_activity;
  };
};

constexpr size_t RETX_QUEUE_SIZE = 4;

constexpr uint8_t kRetransmitLimitMask = 0x07;
constexpr uint8_t kRetransmitStatusMask = 0xe0;
constexpr uint8_t kRetransmitStatusSlotUnused = 0x00;
constexpr uint8_t kRetransmitStatusWaitHashAvail = 0x20;
constexpr uint8_t kRetransmitStatusWaitHashDone = 0x40;
constexpr uint8_t kRetransmitStatusWaitTxSlot = 0x80;
constexpr uint8_t kRetransmitStatusWaitAck = 0xA0;

struct RetransmittableIrPacket {
  uint8_t status;
  // 0x07 - retransmit limit left.
  // 0xe0 - Status
  //   - 0x00 Slot is unused.
  //   - 0x20 Waiting for hashing processor to be available.
  //   - 0x40 Waiting for hashing processor to finish.
  //   - 0x80 Waiting for IrController's tx slot to open up.
  //   - 0xA0 Waiting for Ack.
  uint16_t time_to_retry;
  // In units of IR Retry task calls.
  uint8_t size;
  uint8_t data[MAX_PACKET_PAYLOAD_BYTES + 4];
  uint8_t hash[PACKET_HASH_LEN];
};

class IrController {
 public:
  IrController();

  void Init();
  void ShowText(void* arg);
  void InitBroadcastService(uint8_t game_types);

  void SetDisableBroadcast() { disable_broadcast = true; }

  // Send a packet with data and size len.
  // An acknowledgement is expected (from base station) and will retry if no
  // acknowledgement is received.
  // Return true if the packet is accepted by IrController.
  // Return false if IrController is busy and cannot accept the packet.
  bool SendPacketWithRetransmit(uint8_t* data, size_t len, uint8_t retries);

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

  RetransmittableIrPacket queued_packets_[RETX_QUEUE_SIZE];
  int current_hashing_slot;
  int current_tx_slot;

  // Called every 1s.
  void RoutineTask(void* unused);

  // Called on every packet.
  void OnPacketReceived(void* arg);

  int prob_f(int);

  void BroadcastIr(void* unused);
  void SendShowPacket(char* msg);

  bool TrySendPriority();

  // Periodic check on queued_packets_
  void MaintainQueued();

  // Called when we received an acknowledgement packet.
  void OnAcknowledgePacket(AcknowledgePacket* pckt);
  // Called by HashProcessor when hashing finished.
  void OnPacketHashResult(void* hash_result);
};

extern IrController irController;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
