#ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
#define HITCON_LOGIC_XBOARD_LOGIC_H_

#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

#include <utility>

#include "Service/Sched/Scheduler.h"
#include "Service/XBoardService.h"
#include "XBoardRecvFn.h"
#include "usart.h"

namespace hitcon {
namespace service {
namespace xboard {

struct PacketCallbackArg {
  uint8_t *data;
  uint8_t len;
};

enum UsartConnectState { Init, Connect, Disconnect };

constexpr size_t RX_BUF_SZ = 128;
constexpr size_t PKT_PAYLOAD_LEN_MAX = 32;
constexpr uint8_t PING_TYPE = 208;
constexpr uint8_t PONG_TYPE = 209;

class XBoardLogic {
 public:
  XBoardLogic();

  void Init();

  // Encapsulate data into a packet and then put into the tx queue.
  // Arguments:
  // - `data`: bytes to send, must < `PKT_PAYLOAD_LEN_MAX`
  // - `data_len`: size of the data in bytes
  // - `handler_id`: defined in `fw/Core/Hitcon/Logic/XBoardRecvFn.h`, same as
  // `SetOnPacketArrive`
  void QueueDataForTx(uint8_t *data, uint8_t data_len, RecvFnId handler_id);

  // On detected connection from a remote board, this will be called.
  void SetOnConnect(callback_t callback, void *callback_arg1);

  // On detected disconnection from a remote board, this will be called.
  void SetOnDisconnect(callback_t callback, void *callback_arg1);

  // On received a packet from a remote board, this will be called with a
  // pointer to packet struct.
  // Arguments:
  // - `callback`: function to handle message
  // - `self`: class this or nullptr
  // - `handler_id`: should be the same as `QueueDataForTx`
  void SetOnPacketArrive(callback_t callback, void *self, RecvFnId handler_id);

  enum UsartConnectState GetConnectState();

  bool SendIRPacket(uint8_t *data, size_t len);

 private:
  // buffer variables

  uint8_t rx_buf[RX_BUF_SZ] = {0};
  uint16_t prod_head = 0;
  uint16_t cons_head = 0;
  bool recv_ping = false;
  bool recv_pong = false;
  uint8_t no_pong_count = 0;

  hitcon::service::sched::PeriodicTask _parse_routine;
  hitcon::service::sched::PeriodicTask _ping_routine;
  std::pair<callback_t, void *> packet_arrive_cbs[RecvFnId::MAX] = {};

  UsartConnectState connect_state = UsartConnectState::Init;
  callback_t disconnect_handler = nullptr;
  void *disconnect_handler_self = nullptr;
  callback_t connect_handler = nullptr;
  void *connect_handler_self = nullptr;

  // read `size` bytes from `rx_buf` to `dst`
  // if `head_offset` > 0, start reading from cons_head + head_offset
  // return false if no enough bytes to read
  bool TryReadBytes(uint8_t *dst, size_t size, uint16_t head_offset = 0);
  void SendPing();
  void SendPong();
  void OnByteArrive(void *);
  void ParsePacket();
  void CheckPing();
  void CheckPong();
  void ParseRoutine(void *);
  void PingRoutine(void *);
};

extern XBoardLogic g_xboard_logic;

}  // namespace xboard
}  // namespace service
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
