#ifndef LOGIC_IRCONTROLLER_DOT_H_
#define LOGIC_IRCONTROLLER_DOT_H_

#include <Logic/BadgeId.h>
#include <Logic/EcLogic.h>
#include <Logic/IrLogic.h>
#include <Service/EcParams.h>
#include <Service/IrService.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

enum class packet_type : uint8_t {
  kGame = 0,  // disabled, 2024 Game
  kShow = 1,
  kTest = 2,
  // Packet types for 2025
  kAcknowledge = 3,
  kProximity = 4,            // disabled
  kPubAnnounce = 5,          // disabled
  kTwoBadgeActivity = 6,     // disabled
  kScoreAnnounce = 7,        // disabled
  kSingleBadgeActivity = 8,  // disabled
  kSponsorActivity = 9,      // disabled
  kShowMsg = 10,
  kRequestScore = 11,  // disabled
  kSavePet = 12,       // disabled
  kRestorePet = 13,    // disabled
};

// smaller value means higher priority
// only packets from badge need to have priority.
constexpr uint8_t RETX_LOWEST_PKT_PRIORITY = 0xF0;
constexpr uint8_t RETX_EMPTY_PKT_PRIORITY = 0xF1;
// for kRequestScore and kSavePet, not used now
constexpr uint8_t RETX_REPLACEMENT_PKT_PRIORITY = 0xF2;
constexpr uint8_t packet_priority[14] = {
    RETX_LOWEST_PKT_PRIORITY,  // kGame
    RETX_LOWEST_PKT_PRIORITY,  // kShow
    RETX_LOWEST_PKT_PRIORITY,  // kTest
    RETX_LOWEST_PKT_PRIORITY,  // kAcknowledge
    10,                        // kProximity
    0,                         // kPubAnnounce
    2,                         // kTwoBadgeActivity
    RETX_LOWEST_PKT_PRIORITY,  // kScoreAnnounce
    3,                         // kSingleBadgeActivity
    1,                         // kSponsorActivity
    RETX_LOWEST_PKT_PRIORITY,  // kShowMsg
    4,                         // kRequestScore
    5,                         // kSavePet
    RETX_LOWEST_PKT_PRIORITY   // kRestorePet
};
constexpr uint8_t GetPriority(packet_type type) {
  if (type < packet_type::kGame || type > packet_type::kRestorePet)
    return RETX_LOWEST_PKT_PRIORITY;
  return packet_priority[static_cast<uint8_t>(type)];
}
namespace hitcon {

namespace ir {

/*Definition of IR content.*/

struct ShowPacket {
  char message[16];
};

constexpr size_t PACKET_HASH_LEN = 6;

// This packet acknowledges a particular packet has been received.
struct AcknowledgePacket {
  // Hash of the packet being acknowledge.
  uint8_t packet_hash[PACKET_HASH_LEN];
};

// This packet is from base station to badge.
struct ShowMsgPacket {
  uint8_t user[BADGE_ID_LEN];
  char msg[24];
};

/*Definition of IR content.*/
constexpr size_t IR_DATA_HEADER_SIZE = 2;
struct IrData {
  uint8_t ttl;
  packet_type type;
  // WARNING: Be extra careful about alignment of all struct below, they MUST be
  // made of uint8_t to avoid padding introduced by compiler.
  union {
    struct ShowPacket show;
    struct AcknowledgePacket acknowledge;
    struct ShowMsgPacket show_msg;
  } opaq;
};
static_assert(sizeof(IrData) < 32);

constexpr size_t RETX_QUEUE_SIZE = 4;

constexpr uint8_t kRetransmitLimitMask = 0x07;
constexpr uint8_t kRetransmitStatusMask = 0xe0;
constexpr uint8_t kRetransmitStatusSlotUnused = 0x00;
constexpr uint8_t kRetransmitStatusWaitHashAvail = 0x20;
constexpr uint8_t kRetransmitStatusWaitHashDone = 0x40;
constexpr uint8_t kRetransmitStatusWaitTxSlot = 0x80;
constexpr uint8_t kRetransmitStatusWaitAck = 0xA0;

enum class AckTag : uint8_t {
  ACK_TAG_NONE = 0,
};

struct RetransmittableIrPacket {
  uint8_t status;
  // 0x07 - retransmit limit left.
  // 0xe0 - Status
  //   - 0x00 Slot is unused.
  //   - 0x20 Waiting for hashing processor to be available.
  //   - 0x40 Waiting for hashing processor to finish.
  //   - 0x80 Waiting for IrController's tx slot to open up.
  //   - 0xA0 Waiting for Ack.
  AckTag ack_tag;
  // Ack tag is used internally to denote special events.
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
  void ShowText(char* text);
  void InitBroadcastService(uint8_t game_types);

  void SetDisableBroadcast() { disable_broadcast = true; }

  // Send a packet with data and size len.
  // An acknowledgement is expected (from base station) and will retry if no
  // acknowledgement is received.
  // Return true if the packet is accepted by IrController.
  // Return false if IrController is busy and cannot accept the packet.
  bool SendPacketWithRetransmit(uint8_t* data, size_t len, uint8_t retries,
                                AckTag ack_tag);

  // Query methods for debug interface
  uint8_t GetSlotStatusForDebug(uint8_t slot_index) const;
  uint8_t GetSlotPacketTypeForDebug(uint8_t slot_index) const;
  uint8_t GetSlotRetryCountForDebug(uint8_t slot_index) const;
  uint16_t GetSlotTimeToRetryForDebug(uint8_t slot_index) const;

  void ForceRetransmitForDebug(uint8_t slot_index);

 private:
  bool send_lock = true;
  bool recv_lock = true;
  // TODO: Tune the quadratic function parameters
  uint8_t v[3] = {1, 27, 111};
  bool disable_broadcast = false;

  // Number of packets received, primarily for debugging.
  size_t received_packet_cnt = 0;

  hitcon::service::sched::PeriodicTask routine_task;
  hitcon::service::sched::Task broadcast_task;

  IrData priority_data_;
  size_t priority_data_len_ = 0;

  RetransmittableIrPacket queued_packets_[RETX_QUEUE_SIZE];
  int current_hashing_slot = -1;
  int current_tx_slot = -1;

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

  // Called when we received an acknowledgment packet.
  void OnAcknowledgePacket(AcknowledgePacket* pckt);
  // Called by HashProcessor when hashing finished.
  void OnPacketHashResult(void* hash_result);

  // Called whenever we've some acknowledged packet.
  void OnAcknowledgeTag(AckTag tag);
};

extern IrController irController;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef LOGIC_IRCONTROLLER_DOT_H_
