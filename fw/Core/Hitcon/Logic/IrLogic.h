#ifndef HITCON_LOGIC_IR_LOGIC_H_
#define HITCON_LOGIC_IR_LOGIC_H_

#include <Service/IrParam.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

namespace ir {

struct IrPacket {
  // IR Packet
  // | header | data (1 byte size + n bytes data + 1 byte checksum) |

  IrPacket() : size_(0) {}

  // We need to add 3 bytes because we need
  // at least 1 byte to accomodate the size.
  // at least 1 byte to accomodate the chksum.
  uint8_t data_[MAX_PACKET_PAYLOAD_BYTES + 4];
  size_t size_;
};

class IrLogic {
 public:
  IrLogic();

  // For any actions that needs to be done during init.
  void Init();

  // Every time IR_SERVICE_RX_SIZE byte of buffer is received, this will be
  // called.
  void OnBufferReceived(uint8_t *buffer);

  // Upper layer should call ths function so whenever a well formed packet
  // is received the callback will be called.
  void SetOnPacketReceived(callback_t callback, void *callback_arg1);

  // Send packet with data and size len.
  bool SendPacket(uint8_t *data, size_t len);
  // Return true if it's possible for us to send directly.
  bool AvailableToSend();

  void EncodePacket(uint8_t *data, size_t len, IrPacket &packet);
  // Enqueue the task and reset the counter
  void OnBufferReceivedEnqueueTask(uint8_t *buffer);

  // % of time in last 30 second whereby there's a transmission.
  // 100 => 100%
  // 0 => 0%
  int GetLoadFactor();

  // TODO: check if we need >1 callbacks
  // OnPacketReceived callback
  callback_t callback;
  void *callback_arg;
  IrPacket rx_packet;
  // double buffering to avoid RW same time
  IrPacket rx_packet_ctrler;
  IrPacket tx_packet;

  // This variable is a mystery.
  size_t dummy1 = 0xBAADF00D;

  // Total periods collected for load factor computation.
  size_t lf_total_period;
  // Total periods of non-zero (transmission) collected for load factor
  // computation.
  size_t lf_nonzero_period;

  // Load factor after low pass filter.
  // In Q15.16 fixed point.
  uint32_t lowpass_loadfactor;

  // To split OnBufferReceived into pieces
  size_t buffer_received_ctr;
};

extern IrLogic irLogic;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IR_LOGIC_H_
