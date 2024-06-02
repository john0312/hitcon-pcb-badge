#ifndef HITCON_LOGIC_IR_LOGIC_H_
#define HITCON_LOGIC_IR_LOGIC_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>

namespace hitcon {

constexpr size_t MAX_IR_PACKET_LEN = 32;

class IrPacket {
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
  uint8_t data_[MAX_IR_PACKET_LEN];
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

  // Encodes and transmits a packet.
  void TransmitPacket(uint8_t* packet, size_t len);
};

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_IR_LOGIC_H_
