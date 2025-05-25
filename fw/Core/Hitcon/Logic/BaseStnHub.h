#ifndef LOGIC_BASESTN_HUB_DOT_H_
#define LOGIC_BASESTN_HUB_DOT_H_

#include <stddef.h>
#include <stdint.h>

#include "CdcLogic.h"
#include "Service/Sched/Scheduler.h"
#include "Util/CircularQueue.h"

namespace hitcon {
namespace basestn {

enum BufferType : uint8_t {
  RX = 0,
  TX = 1,
};

struct BufferMeta {
  // this bit is set after the buffer is written, it is cleared after the buffer
  // is read
  uint8_t locked : 1;
  // length of the data in the buffer
  uint8_t length : 7;
};

constexpr size_t kBufferSize = 33;  // 1 byte for status, 32 bytes for data
constexpr size_t kBufferCount = 4;  // 4 RX and 4 TX buffers
constexpr size_t kTotalBufferSize = kBufferSize * kBufferCount;

static_assert(sizeof(BufferMeta) == 1,
              "SimpleBufferIndex must be exactly 1 byte");

// Forwards packet between usb and ir interface.
// There are tx buffers and rx buffers, there are 4 buffers of each, so there
// are 8 buffers in total.
// Each buffer is 33 bytes, 1 byte for status and 32 byte for data.
// The status byte is managed by the callers, we don't care about it.
// The external writers/readers should get the status and length from the first
// byte.
class BaseStationHub {
 public:
  BaseStationHub();
  void Init();

  // buffer index specification:
  // [0:7] Index into the buffer.
  // [8:12] Reserved. Will be ignored by methods in this class.
  // [13:14] Which buffer? 0 to 3.
  // [15] Set for TX, clear for RX.

  // Try to write to an available buffer. If there is no available buffer,
  // return false.
  bool WriteBuffer(BufferType buffer_type, uint8_t* data, size_t cnt);

  // Read from any available buffer. If there is no available buffer, return
  // false. If the buffer is read, the length is set to cnt.
  bool ReadBuffer(BufferType buffer_type, uint8_t* data, size_t& cnt);

  void OnIrPacketRecv(uint8_t* data, size_t cnt);

 private:
  // 4 rx buffers and 4 tx buffers.
  // Each buffer is 33 bytes, 1 byte for status and 32 byte for data.
  uint8_t rx_buffer[kTotalBufferSize] = {0};
  uint8_t tx_buffer[kTotalBufferSize] = {0};
  CircularQueue<uint8_t, kBufferCount> rxq;
  CircularQueue<uint8_t, kBufferCount> txq;

  hitcon::service::sched::PeriodicTask _routine_task;
  void QueueTxHandler(hitcon::logic::cdc::PacketCallbackArg* arg);

  void Routine(void*);
  void SendToIr();
  void SendToBaseStation();
};

extern BaseStationHub g_basestn_hub;

}  // namespace basestn
}  // namespace hitcon

#endif  // #ifndef LOGIC_BASESTN_HUB_DOT_H_
