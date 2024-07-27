#ifndef HITCON_LOGIC_IR_LOGIC_H_
#define HITCON_LOGIC_IR_LOGIC_H_

#include <Service/IrParam.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

namespace ir {

struct IrPacket {
	IrPacket() : size_(0) {};

	// We need to add 2 bytes because we need at least 1 byte to accomodate
	// the size.
	uint8_t data_[MAX_PACKET_PAYLOAD_BYTES+2];
	size_t size_;
};

class IrLogic {
 public:
  IrLogic();

  // For any actions that needs to be done during init.
  void Init();

  // Every time IR_SERVICE_RX_SIZE is received, this will be called.
  void OnBufferReceived(uint8_t *buffer);

  // Upper layer should call ths function so whenever a well formed packet
  // is received the callback will be called.
  void SetOnPacketReceived(callback_t callback, void *callback_arg1);

  bool SendPacket(uint8_t *data, size_t len);

  void EncodePacket(uint8_t *data, size_t len, IrPacket &packet);

  // % of time in last 30 second whereby there's a transmission.
  // 10000 => 100%
  // 0 => 0%
  int GetLoadFactor();

  // TODO: check if we need >1 callbacks
  // OnPacketReceived callback
  callback_t callback;
  void *callback_arg;
  IrPacket rx_packet;
  IrPacket tx_packet;
};

extern IrLogic irLogic;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IR_LOGIC_H_
