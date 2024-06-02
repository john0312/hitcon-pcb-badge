#ifndef HITCON_SERVICE_XBOARD_SERVICE_H_
#define HITCON_SERVICE_XBOARD_SERVICE_H_

#include <stdint.h>
#include <stddef.h>
#include <Util/callback.h>

namespace hitcon {

constexpr size_t MAX_XBOARD_PACKET_LEN = 32;

class XBoardPacket {
 public:
  XBoardPacket() : size_(0) {};

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
  uint8_t data_[MAX_XBOARD_PACKET_LEN];
  size_t size_;
};

class XBoardService {
 public:
  XBoardService();

  // Call QueuePacketForTx() to queue a packet for transmission on
  // the XBoard connection.
  void QueuePacketForTx(uint8_t* packet, size_t packet_len);

  // On detected connection from a remote board, this will be called.
  void SetOnConnectCallback(callback_t callback, void* callback_arg1);

  // On detected disconnection from a remote board, this will be called.
  void SetOnDisconnectCallback(callback_t callback, void* callback_arg1);

  // On received a packet from a remote board, this will be called with a
  // pointer to packet struct.
  void SetOnPacketCallback(callback_t callback, void* callback_arg1);

 private:
  XBoardPacket packet_buffer[4];
};

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_XBOARD_SERVICE_H_
