#ifndef HITCON_LOGIC_IR_LOGIC_H_
#define HITCON_LOGIC_IR_LOGIC_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>

#define PACKET_START                                                           \
  (uint8_t[]) { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 }
#define PACKET_START_LEN (sizeof(PACKET_START) / sizeof(uint8_t))
#define PACKET_SIZE_LEN 5
#define PACKET_DATA_MAX_BYTE 32
#define PACKET_DATA_MAX_BIT (PACKET_DATA_MAX_BYTE * 8)
#define PACKET_PARITY_LEN 1
#define PACKET_END                                                             \
  (uint8_t[]) { 1, 1, 1, 1, 1, 1, 1 }
#define PACKET_END_LEN (sizeof(PACKET_END) / sizeof(uint8_t))
#define PACKET_MAX_LEN                                                         \
  (PACKET_START_LEN + PACKET_SIZE_LEN + PACKET_DATA_MAX_BIT +                  \
   PACKET_PARITY_LEN + PACKET_END_LEN)

// This value means that the hardware samples DECODE_SAMPLE_RATIO times within a
// bit-time. Higher value reduces the chance of hardware error but increases the
// time to decode a packet.
#define DECODE_SAMPLE_RATIO 4
// The number of 0s or 1s to be considered as a valid bit. If the number of 0s
// and 1s are both less than this value, the bit is considered as invalid.
#define DECODE_SAMPLE_RATIO_THRESHOLD 3

namespace hitcon {

class IrPacket {
 friend class IrLogic;
 public:
	IrPacket() : size_(0) {};

	uint8_t* data() {
		return data_;
	}

	size_t size() {
		return size_;
	}

	void set_size(size_t s) {
		size_ = s;
	};

 private:
	uint8_t data_[PACKET_MAX_LEN];
	size_t size_;
};

class IrLogic
{
 public:
  IrLogic();

  // For any actions that needs to be done during init.
  void Init();

  // Every time IR_SERVICE_RX_SIZE is received, this will be called.
  void OnBufferReceived(uint8_t* buffer);

  // Upper layer should call ths function so whenever a well formed packet
  // is received the callback will be called.
  void SetOnPacketReceived(callback_t callback, void* callback_arg1);

  void EncodePacket(uint8_t *data, size_t len, IrPacket &packet);
  int DecodePacket(IrPacket &packet, size_t *len, uint8_t *decodedData);
};

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IR_LOGIC_H_
