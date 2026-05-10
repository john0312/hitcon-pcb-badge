#ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
#define HITCON_LOGIC_XBOARD_LOGIC_H_

#include <Hitcon.h>
#include <Util/CircularQueue.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

#include <utility>

#include "Service/Sched/Scheduler.h"
#include "Service/XBoardService.h"
#include "XBoardRecvFn.h"
#include "usart.h"

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

namespace hitcon {
namespace service {
namespace xboard {

struct PacketCallbackArg {
  uint8_t *data;
  uint8_t len;
};

enum class PeerType : uint8_t {
  None,
  Legacy,
  Peer2025,
  BaseStn2025,
  QRStn2026,
  NUM_PEER_TYPES,
};

constexpr size_t RX_BUF_SZ = 128;
constexpr size_t PKT_PAYLOAD_LEN_MAX = 32;
constexpr uint8_t PING_TYPE = 208;
constexpr uint8_t PONG_LEGACY_TYPE = 209;
constexpr uint8_t PONG_PEER2025_TYPE = 210;
constexpr uint8_t PONG_BASESTN2025_TYPE = 211;
constexpr uint8_t PONG_QRSTN2026_TYPE = 212;

#if BADGE_ROLE == BADGE_ROLE_QR_STN
constexpr uint8_t SELF_PONG_TYPE = PONG_QRSTN2026_TYPE;
#else
constexpr uint8_t SELF_PONG_TYPE = PONG_PEER2025_TYPE;
#endif

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
  void QueueDataForTx(const uint8_t *data, uint8_t data_len,
                      RecvFnId handler_id);

  // Register callback for when a connection to `peer` is detected.
  // `peer` must not be `PeerType::None`.
  void SetOnConnect(PeerType peer, callback_t callback, void *callback_arg1);

  // Register callback for when a disconnection from `peer` is detected.
  // `peer` must not be `PeerType::None`.
  void SetOnDisconnect(PeerType peer, callback_t callback, void *callback_arg1);

  // On received a packet from a remote board, this will be called with a
  // pointer to packet struct.
  // Arguments:
  // - `callback`: function to handle message
  // - `self`: class this or nullptr
  // - `handler_id`: should be the same as `QueueDataForTx`
  void SetOnPacketArrive(callback_t callback, void *self, RecvFnId handler_id);

  PeerType GetPeer() const { return peer; }
  bool IsConnected() const { return peer != PeerType::None; }

 private:
  // buffer variables

  CircularQueue<uint8_t, RX_BUF_SZ> rx_queue;
  bool recv_ping = false;
  uint8_t recv_pong_flags = 0;
  // 0x01 - Legacy pong received.
  // 0x02 - Peer 2025 pong received.
  // 0x04 - Base station pong received.
  // 0x08 - QR station 2026 pong received.
  uint8_t no_pong_count = 0;

  hitcon::service::sched::PeriodicTask _parse_routine;
  hitcon::service::sched::PeriodicTask _ping_routine;
  std::pair<callback_t, void *> packet_arrive_cbs[RecvFnId::MAX] = {};

  PeerType peer = PeerType::None;
  std::pair<callback_t, void *>
      connect_cbs[static_cast<size_t>(PeerType::NUM_PEER_TYPES)] = {};
  std::pair<callback_t, void *>
      disconnect_cbs[static_cast<size_t>(PeerType::NUM_PEER_TYPES)] = {};

  void SendPing();
  void SendPeerPong();
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
