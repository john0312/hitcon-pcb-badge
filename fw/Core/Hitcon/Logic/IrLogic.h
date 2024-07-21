#ifndef HITCON_LOGIC_IR_LOGIC_H_
#define HITCON_LOGIC_IR_LOGIC_H_

#include <Service/IrParam.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

namespace ir {

// Ring buffer
struct queue_t {
  uint8_t buf[QUEUE_MAX_SIZE];
  size_t start, end;  // data is in [start, end), circular buffer
  inline size_t size() {
    return (QUEUE_MAX_SIZE + end - start) % QUEUE_MAX_SIZE;
  }
  inline void push(uint8_t ch) {
    if (QUEUE_MAX_SIZE == end + 1) {
      buf[0] = ch;
      end = 0;
    } else {
      buf[end++] = ch;
    }
  }
};

struct IrPacket {
	IrPacket() : size_(0) {};

  // TODO: later eliminate these
	uint8_t* data() {
		return data_;
	}

  size_t size() { return size_; }

  void set_size(size_t s) { size_ = s; };

	uint8_t data_[PACKET_MAX_LEN];
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
  int DecodePacket(IrPacket &packet, size_t *len, uint8_t *decodedData);

  // % of time in last 30 second whereby there's a transmission.
  // 10000 => 100%
  // 0 => 0%
  int GetLoadFactor();

  // ring buffer
  queue_t packet_queue;

  // TODO: check if we need >1 callbacks
  // OnPacketReceived callback
  callback_t callback;
  void *callback_arg;
};

extern IrLogic irLogic;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IR_LOGIC_H_
