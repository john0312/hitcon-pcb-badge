#include "IrController.h"

#include <App/HardwareTestApp.h>
#include <App/ShowNameApp.h>
#include <App/TamaApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameController.h>
#include <Logic/IrController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>
#include <Service/HashService.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <Service/SignedPacketService.h>
#include <stdlib.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::UsartConnectState;

namespace hitcon {
namespace ir {

namespace {

static char SURPRISE_NAME[] = "You got pwned!";

}  // anonymous namespace

IrController irController;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
IrController::IrController()
    : routine_task(950, (callback_t)&IrController::RoutineTask, this, 1000),
      broadcast_task(800, (callback_t)&IrController::BroadcastIr, this) {}
#pragma GCC diagnostic pop

void IrController::ShowText(char* text) {
  badge_controller.SetStoredApp(badge_controller.GetCurrentApp());
  show_name_app.SetSurpriseMsg(text);
  show_name_app.SetMode(Surprise);
  badge_controller.change_app(&show_name_app);
}

void IrController::Init() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  irLogic.SetOnPacketReceived((callback_t)&IrController::OnPacketReceived,
                              this);
  badge_controller.SetCallback((callback_t)&IrController::SendShowPacket, this,
                               SURPRISE_NAME);
#pragma GCC diagnostic pop
  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void IrController::OnPacketReceived(void* arg) {
  received_packet_cnt++;

  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);
  IrData* data = reinterpret_cast<IrData*>(&packet->data_[1]);

  // Game
  if (data->type == packet_type::kGame) {
    // removed
  } else if (data->type == packet_type::kTest) {
    hardware_test_app.CheckIr(&data->opaq.show);
  } else if (data->type == packet_type::kShow) {
    ShowText(data->opaq.show.message);
  } else if (data->type == packet_type::kShowMsg) {
    const uint8_t* user = g_game_controller.GetUsername();
    if (user &&
        (memcmp(data->opaq.show_msg.user, user, IR_USERNAME_LEN) == 0)) {
      ShowText(data->opaq.show_msg.msg);
    }
  } else if (data->type == packet_type::kAcknowledge) {
    OnAcknowledgePacket(&data->opaq.acknowledge);
  } else if (data->type == packet_type::kScoreAnnounce) {
    const uint8_t* user = g_game_controller.GetUsername();
    if (user &&
        memcmp(data->opaq.score_announce.user, user, IR_USERNAME_LEN) == 0) {
      g_signed_packet_service.VerifyAndReceivePacket(packet);
    } else {
      // Not our score.
    }
  } else if (data->type == packet_type::kRestorePet) {
    const uint8_t* user = g_game_controller.GetUsername();
    if (user &&
        memcmp(data->opaq.restore_pet.user, user, IR_USERNAME_LEN) == 0) {
      g_signed_packet_service.VerifyAndReceivePacket(packet);
    } else {
      // Not our pet.
    }
  }
}

void IrController::OnAcknowledgePacket(AcknowledgePacket* pckt) {
  for (int i = 0; i < RETX_QUEUE_SIZE; i++) {
    uint8_t status = queued_packets_[i].status;
    if ((status & kRetransmitStatusMask) == kRetransmitStatusWaitTxSlot ||
        (status & kRetransmitStatusMask) == kRetransmitStatusWaitAck) {
      if (memcmp(queued_packets_[i].hash, pckt->packet_hash, PACKET_HASH_LEN) ==
          0) {
        AckTag ack = queued_packets_[i].ack_tag;
        OnAcknowledgeTag(ack);
        // Received, no longer need to retransmit.
        queued_packets_[i].status =
            (queued_packets_[i].status & (~kRetransmitStatusMask));
      }
    }
  }
}

void IrController::OnAcknowledgeTag(AckTag tag) {
  // Hardcoded receivers.
  switch (tag) {
    case AckTag::ACK_TAG_NONE:
      return;
    case AckTag::ACK_TAG_PUBKEY_RECOG:
      g_game_controller.NotifyPubkeyAck();
      return;
  }
}

int IrController::prob_f(int lf) { return v[0] * lf * lf + v[1] * lf + v[2]; }

void IrController::RoutineTask(void* unused) {
  // remove generating random number
  MaintainQueued();
}

void IrController::OnPacketHashResult(void* arg_ptr) {
  my_assert(current_hashing_slot != -1);
  my_assert(current_hashing_slot < RETX_QUEUE_SIZE);
  hitcon::hash::HashResult* hash_result =
      reinterpret_cast<hitcon::hash::HashResult*>(arg_ptr);
  memcpy(&(queued_packets_[current_hashing_slot].hash[0]), hash_result->digest,
         PACKET_HASH_LEN);
  my_assert(PACKET_HASH_LEN <= hash_result->size);
  uint8_t status = queued_packets_[current_hashing_slot].status;
  // Update status to Waiting for IrController's tx slot
  status = (status & (~kRetransmitStatusMask)) | kRetransmitStatusWaitTxSlot;
  queued_packets_[current_hashing_slot].status =
      status;  // Update the struct member
  current_hashing_slot = -1;
}

// - if there are same packet_type: replaced with the new one first (only
// Proximity, RequestScore, SavePet)
// - else: replace the empty slot
// - else: find the lowest priority (lower than input priority) slot to replace
bool IrController::SendPacketWithRetransmit(uint8_t* data, size_t len,
                                            uint8_t retries, AckTag ack_tag) {
  my_assert(len <= MAX_PACKET_PAYLOAD_BYTES);
  my_assert(retries < 8);  // Max retries fits in 3 bits
  const IrData* input_ir_data = reinterpret_cast<IrData*>(data);
  uint8_t input_priority = GetPriority(input_ir_data->type);
  uint8_t lowest_priority = input_priority;
  uint8_t available_index = RETX_QUEUE_SIZE;
  for (int i = 0; i < RETX_QUEUE_SIZE; i++) {
    const IrData* queued_ir_data =
        reinterpret_cast<IrData*>(queued_packets_[i].data);
    uint8_t queue_priority = GetPriority(queued_ir_data->type);

    if (queued_ir_data->type == input_ir_data->type &&
        (input_ir_data->type == packet_type::kProximity ||
         input_ir_data->type == packet_type::kRequestScore ||
         input_ir_data->type == packet_type::kSavePet)) {
      available_index = i;
      break;
    }

    // Check if slot is empty by checking the status mask
    if ((queued_packets_[i].status & kRetransmitStatusMask) ==
        kRetransmitStatusSlotUnused) {
      // Slot is empty.
      queue_priority = LOWEST_PRIORITY;
    }

    if (lowest_priority < queue_priority) {
      // find the lowest priority slot to replace
      available_index = i;
      lowest_priority = queue_priority;
    }
  }

  if (available_index < RETX_QUEUE_SIZE) {
    // stop hash if the replaced slot is waiting for hash done
    if ((queued_packets_[available_index].status & kRetransmitStatusMask) ==
        kRetransmitStatusWaitHashDone) {
      hash::g_hash_service.StopHash();
    }

    memcpy(&(queued_packets_[available_index].data[0]), data, len);
    queued_packets_[available_index].size = len;
    // Set status to Waiting for hashing processor and store retry limit
    queued_packets_[available_index].status =
        kRetransmitStatusWaitHashAvail | (retries & kRetransmitLimitMask);
    queued_packets_[available_index].ack_tag = ack_tag;
    return true;
  }
  return false;  // No available slot found
}

void IrController::MaintainQueued() {
  if (irLogic.AvailableToSend()) {
    current_tx_slot = -1;
  }

  // Iterate through the queue slots in a randomized order.
  for (int j = 0; j < RETX_QUEUE_SIZE; j++) {
    // Randomize the slot index to avoid always checking/sending from the same
    // slots first. Sending from the same slots first may result in scheduling
    // starvation.
    int i = (j + (g_fast_random_pool.GetRandom() % RETX_QUEUE_SIZE)) %
            RETX_QUEUE_SIZE;

    // Get the current status and other packet info.
    uint8_t current_status = queued_packets_[i].status & kRetransmitStatusMask;
    uint8_t pckt_size = queued_packets_[i].size;
    // This is the payload data within the struct.
    uint8_t* pckt_data = &(queued_packets_[i].data[0]);

    if (current_status == kRetransmitStatusSlotUnused) {
      // Slot is unused. Do nothing.
    } else if (current_status == kRetransmitStatusWaitHashAvail) {
      // Waiting for hash processor to be available.
      if (current_hashing_slot == -1) {
        // Start hashing the payload.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
        bool ret = hitcon::hash::g_hash_service.StartHash(
            pckt_data, pckt_size, (callback_t)&IrController::OnPacketHashResult,
            this);
#pragma GCC diagnostic pop
        if (ret) {
          // Hashing started successfully. Update status to Waiting for hash
          // processor to finish.
          queued_packets_[i].status =
              (queued_packets_[i].status & ~kRetransmitStatusMask) |
              kRetransmitStatusWaitHashDone;
          current_hashing_slot =
              i;  // Mark this slot as being currently hashed.
        }
        // If ret is false, hash service was busy, will try again next
        // RoutineTask cycle.
      }
    } else if (current_status == kRetransmitStatusWaitHashDone) {
      // Waiting for hash processor to finish.
      // The OnPacketHashResult callback will change the status to
      // kRetransmitStatusWaitTxSlot once hashing is complete and the hash is
      // stored. Do nothing here.
    } else if (current_status == kRetransmitStatusWaitTxSlot) {
      // Waiting for IrController's tx slot to open up. (Hash is ready)
      if (current_tx_slot == -1) {
        bool ret;
        if (g_xboard_logic.GetConnectState() ==
            UsartConnectState::ConnectBaseStn2025) {
          ret = g_xboard_logic.SendIRPacket(&(queued_packets_[i].data[0]),
                                            queued_packets_[i].size);
        } else {
          ret = irLogic.SendPacket(&(queued_packets_[i].data[0]),
                                   queued_packets_[i].size);
        }
        if (ret) {
          // Packet successfully queued for transmission by irLogic.
          current_tx_slot =
              i;  // Mark this slot as currently being transmitted.
          // Update status to Waiting for ACK.
          queued_packets_[i].status =
              (queued_packets_[i].status & ~kRetransmitStatusMask) |
              kRetransmitStatusWaitAck;
          // Set the timer for waiting for an acknowledgment packet.
          queued_packets_[i].time_to_retry =
              600 + 200 - (g_fast_random_pool.GetRandom() % 400);
        }
        // If ret is false, irLogic was busy, will try again next RoutineTask
        // cycle.
      }
    } else if (current_status == kRetransmitStatusWaitAck) {
      // Waiting for ACK. Check the retry timer.
      if (queued_packets_[i].time_to_retry == 0) {
        // Timer elapsed, no ACK received. Check if retries are left.
        uint8_t counts = queued_packets_[i].status &
                         kRetransmitLimitMask;  // Get remaining retry count.
        if (counts == 0) {
          // No more retries left. Mark this slot as unused.
          queued_packets_[i].status = kRetransmitStatusSlotUnused;
        } else {
          // Retries left. Decrement the count and transition back to waiting
          // for TX slot.
          counts--;
          // Preserve the new count and retransmit.
          queued_packets_[i].status =
              (queued_packets_[i].status & ~kRetransmitLimitMask) |
              counts;  // Update retry count.
          queued_packets_[i].status =
              (queued_packets_[i].status & ~kRetransmitStatusMask) |
              kRetransmitStatusWaitTxSlot;  // Update status.
        }
      } else {
        // Timer is still counting down. Decrement it.
        queued_packets_[i].time_to_retry--;
      }
    }
  }
}

void IrController::BroadcastIr(void* unused) {
  if (disable_broadcast) return;

  if (!TrySendPriority()) return;

  // remove broadcasting
}

void IrController::SendShowPacket(char* msg) {
  IrData irdata = {
      .ttl = 0,
      .type = packet_type::kShow,
  };
  size_t length = strlen(msg);
  memcpy(irdata.opaq.show.message, msg, length);
  memcpy(&priority_data_, &irdata, sizeof(irdata));
  priority_data_len_ = sizeof(priority_data_) / sizeof(uint8_t);
  ;
  TrySendPriority();
}

bool IrController::TrySendPriority() {
  if (priority_data_len_ == 0) return true;

  uint8_t irdata_len = sizeof(priority_data_) / sizeof(uint8_t);
  bool ret = irLogic.SendPacket(reinterpret_cast<uint8_t*>(&priority_data_),
                                irdata_len);
  if (ret) {
    priority_data_len_ = 0;
  }
  return false;
}

uint8_t IrController::GetSlotStatusForDebug(uint8_t slot_index) const {
  if (slot_index >= RETX_QUEUE_SIZE) return 0;
  return queued_packets_[slot_index].status & kRetransmitStatusMask;
}

uint8_t IrController::GetSlotPacketTypeForDebug(uint8_t slot_index) const {
  if (slot_index >= RETX_QUEUE_SIZE) return 0;
  uint8_t status_mask =
      queued_packets_[slot_index].status & kRetransmitStatusMask;
  if (status_mask == kRetransmitStatusSlotUnused) return 0;

  // The packet type is stored in the second byte of the data
  // (after the TTL byte in IrData structure)
  return static_cast<uint8_t>(queued_packets_[slot_index].data[1]);
}

uint8_t IrController::GetSlotRetryCountForDebug(uint8_t slot_index) const {
  if (slot_index >= RETX_QUEUE_SIZE) return 0;
  return queued_packets_[slot_index].status & kRetransmitLimitMask;
}

uint16_t IrController::GetSlotTimeToRetryForDebug(uint8_t slot_index) const {
  if (slot_index >= RETX_QUEUE_SIZE) return 0;
  return queued_packets_[slot_index].time_to_retry;
}

void IrController::ForceRetransmitForDebug(uint8_t slot_index) {
  if (slot_index >= RETX_QUEUE_SIZE) return;
  if ((queued_packets_[slot_index].status & kRetransmitStatusMask) ==
      kRetransmitStatusWaitAck) {
    uint8_t counts = queued_packets_[slot_index].status & kRetransmitLimitMask;
    if (counts <= 5) {
      counts++;
    }
    queued_packets_[slot_index].status =
        kRetransmitStatusWaitTxSlot | (counts & kRetransmitLimitMask);
  }
}

}  // namespace ir
}  // namespace hitcon
