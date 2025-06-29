#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Logic/crc32.h>
#include <Service/Suspender.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;

namespace hitcon {
namespace service {
namespace xboard {

XBoardLogic g_xboard_logic;

namespace {
inline uint16_t inc_head(size_t head, size_t offset) {
  return (head + offset) % RX_BUF_SZ;
}
constexpr uint8_t PADDING_MAP[] = {0, 3, 2, 1};
}  // namespace

constexpr uint64_t PREAMBLE = 0xD555555555555555ULL;
struct Frame {
  uint64_t preamble;  // 0xD555555555555555
  uint16_t id;
  uint8_t len;   // should < `PKT_PAYLOAD_LEN_MAX`
  uint8_t type;  // 208(0xd0): ping
  uint32_t checksum;
};
constexpr size_t HEADER_SZ = sizeof(Frame);

// public functions

XBoardLogic::XBoardLogic()
    : _parse_routine(490, (task_callback_t)&XBoardLogic::ParseRoutine, this,
                     20),
      _ping_routine(490, (task_callback_t)&XBoardLogic::PingRoutine, this,
                    200) {}

void XBoardLogic::Init() {
  scheduler.Queue(&_parse_routine, nullptr);
  scheduler.EnablePeriodic(&_parse_routine);
  scheduler.Queue(&_ping_routine, nullptr);
  scheduler.EnablePeriodic(&_ping_routine);
  g_xboard_service.SetOnByteRx((callback_t)&XBoardLogic::OnByteArrive, this);
}

void XBoardLogic::QueueDataForTx(uint8_t *packet, uint8_t packet_len,
                                 RecvFnId handler_id) {
  my_assert(packet_len < PKT_PAYLOAD_LEN_MAX);
  uint8_t pkt[HEADER_SZ + PKT_PAYLOAD_LEN_MAX] = {0};
  *(Frame *)pkt =
      Frame{PREAMBLE, 0, packet_len, static_cast<uint8_t>(handler_id), 0};
  for (uint8_t i = 0; i < packet_len; ++i) {
    pkt[i + HEADER_SZ] = packet[i];
  }
  reinterpret_cast<Frame *>(pkt)->checksum =
      fast_crc32(pkt, HEADER_SZ + packet_len + PADDING_MAP[packet_len & 0b11]);
  g_xboard_service.QueueDataForTx(pkt, HEADER_SZ + packet_len);
}

void XBoardLogic::SetOnConnectLegacy(callback_t callback, void *self) {
  connect_legacy_handler = callback;
  connect_legacy_handler_self = self;
}

void XBoardLogic::SetOnDisconnectLegacy(callback_t callback, void *self) {
  disconnect_legacy_handler = callback;
  disconnect_legacy_handler_self = self;
}

void XBoardLogic::SetOnConnectPeer2025(callback_t callback, void *self) {
  connect_peer2025_handler = callback;
  connect_peer2025_handler_self = self;
}

void XBoardLogic::SetOnDisconnectPeer2025(callback_t callback, void *self) {
  disconnect_peer2025_handler = callback;
  disconnect_peer2025_handler_self = self;
}

void XBoardLogic::SetOnConnectBaseStn2025(callback_t callback, void *self) {
  connect_basestn2025_handler = callback;
  connect_basestn2025_handler_self = self;
}

void XBoardLogic::SetOnDisconnectBaseStn2025(callback_t callback, void *self) {
  disconnect_basestn2025_handler = callback;
  disconnect_basestn2025_handler_self = self;
}

void XBoardLogic::SetOnPacketArrive(callback_t callback, void *self,
                                    RecvFnId handler_id) {
  packet_arrive_cbs[handler_id] = {callback, self};
}

// private functions

bool XBoardLogic::TryReadBytes(uint8_t *dst, size_t size,
                               uint16_t head_offset) {
  uint16_t _cons_head = inc_head(cons_head, head_offset);
  if (cons_head == prod_head) {
    if (_cons_head != cons_head) return false;
  } else if (cons_head < prod_head) {
    // Ok: c <= _c <= p
    if (_cons_head > prod_head) return false;
    if (_cons_head < cons_head) return false;
  } else {
    // Ok: p < c <= _c
    // Ok: _c <= p < c
    if (prod_head < _cons_head && _cons_head < cons_head) return false;
  }
  uint16_t remain_size =
      (_cons_head > prod_head ? 0 : RX_BUF_SZ) - prod_head + _cons_head;
  if (remain_size < size) {
    return false;
  }
  uint16_t next_cons_head = inc_head(_cons_head, size);
  if (next_cons_head < _cons_head) {
    uint16_t sz1 = RX_BUF_SZ - _cons_head;
    memcpy(dst, rx_buf + _cons_head, sz1);
    memcpy(dst + sz1, rx_buf, size - sz1);
  } else {
    memcpy(dst, rx_buf + _cons_head, size);
  }
  return true;
}

bool XBoardLogic::SendIRPacket(uint8_t *data, size_t len) {
  g_xboard_logic.QueueDataForTx(data, len, IR_RECV_ID);
  // TODO: Checking ACK
  // assuming always ACKed now
  return true;
}

void XBoardLogic::SendPing() {
  uint8_t pkt[HEADER_SZ] = {0};
  *reinterpret_cast<Frame *>(pkt) = Frame{PREAMBLE, 0, 0, PING_TYPE, 0};
  reinterpret_cast<Frame *>(pkt)->checksum = fast_crc32(pkt, HEADER_SZ);
  // for (int i = 0; i < sizeof(Frame); i++) {
  //   pkt[i] = (0x11+i)&0x0FF;
  //   pkt[i] = 0;
  // }
  g_xboard_service.QueueDataForTx(pkt, sizeof(pkt));
}

void XBoardLogic::SendPeerPong() {
  uint8_t pkt[HEADER_SZ] = {0};
  *reinterpret_cast<Frame *>(pkt) =
      Frame{PREAMBLE, 0, 0, PONG_PEER2025_TYPE, 0};
  reinterpret_cast<Frame *>(pkt)->checksum = fast_crc32(pkt, HEADER_SZ);
  // for (int i = 0; i < sizeof(Frame); i++) {
  //   pkt[i] = (0x11+i)&0x0FF;
  //   pkt[i] = 0;
  // }
  g_xboard_service.QueueDataForTx(pkt, sizeof(pkt));
}

void XBoardLogic::OnByteArrive(void *arg1) {
  uint8_t b = static_cast<uint8_t>(reinterpret_cast<size_t>(arg1));
  uint16_t next_prod_head = inc_head(prod_head, 1);
  if (next_prod_head == cons_head) {
    // drop the data
    AssertOverflow();
    return;
  }
  rx_buf[prod_head] = b;
  prod_head = next_prod_head;
}

void XBoardLogic::ParsePacket() {
  size_t bytes_processed = 0;
  while (cons_head != prod_head && bytes_processed < 16) {
    if (rx_buf[cons_head] != 0x55) {
      cons_head = inc_head(cons_head, 1);
      ++bytes_processed;
      continue;
    }
    uint16_t in_buf_size =
        (prod_head > cons_head ? 0 : RX_BUF_SZ) + prod_head - cons_head;
    if (in_buf_size < HEADER_SZ) {
      break;
    }

    uint8_t pkt[HEADER_SZ + PKT_PAYLOAD_LEN_MAX] = {0};
    Frame *header = reinterpret_cast<Frame *>(pkt);
    uint8_t *payload = pkt + HEADER_SZ;
    TryReadBytes(reinterpret_cast<uint8_t *>(header), HEADER_SZ);
    if (header->preamble != PREAMBLE) {
      cons_head = inc_head(cons_head, 1);
      ++bytes_processed;
      continue;
    }
    if (header->len >= PKT_PAYLOAD_LEN_MAX) {
      // invalid packet, skip this packet (preamble 8 bytes)
      cons_head = inc_head(cons_head, 8);
      bytes_processed += 8;
      continue;
    }
    if (!TryReadBytes(payload, header->len, HEADER_SZ)) {
      // no enough bytes to read, wait more bytes in
      return;
    }

    uint32_t recv_check = header->checksum;
    header->checksum = 0;
    if (fast_crc32(pkt, HEADER_SZ + header->len +
                            PADDING_MAP[header->len & 0b11]) != recv_check) {
      cons_head = inc_head(cons_head, 8);
      bytes_processed += 8;
      continue;
    }

    // pass checking, valid packet now
    cons_head = inc_head(cons_head, HEADER_SZ + header->len);
    if (header->type == PING_TYPE) {
      recv_ping = true;
      continue;
    }
    if (header->type == PONG_LEGACY_TYPE) {
      recv_pong_flags |= 0x01;
      continue;
    }
    if (header->type == PONG_PEER2025_TYPE) {
      recv_pong_flags |= 0x02;
      continue;
    }
    if (header->type == PONG_BASESTN2025_TYPE) {
      recv_pong_flags |= 0x04;
      continue;
    }

    // app callbacks
    if (header->type < RecvFnId::MAX) {
      PacketCallbackArg packet_cb_arg;
      packet_cb_arg.data = payload;
      packet_cb_arg.len = header->len;
      auto [recv_fn, recv_self] = packet_arrive_cbs[header->type];
      if (recv_fn != nullptr) recv_fn(recv_self, &packet_cb_arg);
    }
    // handle at most one packet each time
    break;
  }
}

void XBoardLogic::CheckPing() {
  if (recv_ping) {
    SendPeerPong();
  }
  recv_ping = false;
}

void XBoardLogic::CheckPong() {
  UsartConnectState next_state = connect_state;
  if (recv_pong_flags == 0x01) {
    // Received legacy pong.
    next_state = UsartConnectState::ConnectLegacy;
    no_pong_count = 0;
  } else if (recv_pong_flags == 0x02) {
    // Received peer 2025 pong.
    next_state = UsartConnectState::ConnectPeer2025;
    no_pong_count = 0;
  } else if (recv_pong_flags == 0x04) {
    // Received base stn 2025 pong.
    next_state = UsartConnectState::ConnectBaseStn2025;
    no_pong_count = 0;
  } else {
    // recv_pong_flags == 0 or some combination, either way
    // No pong at all.
    if (connect_state == UsartConnectState::Init) no_pong_count = 3;
    if (no_pong_count < 3) {
      ++no_pong_count;
    }
    if (no_pong_count >= 3) {
      next_state = UsartConnectState::Disconnect;
    }
  }
  if (next_state != connect_state) {
    if (next_state == UsartConnectState::Disconnect &&
        connect_state != UsartConnectState::Init) {
      g_suspender.DecBlocker();
    } else if (next_state == UsartConnectState::ConnectBaseStn2025 ||
               next_state == UsartConnectState::ConnectLegacy ||
               next_state == UsartConnectState::ConnectPeer2025) {
      g_suspender.IncBlocker();
    }
  }

  if (next_state != connect_state && connect_state != UsartConnectState::Init) {
    if (connect_state == UsartConnectState::ConnectLegacy &&
        disconnect_legacy_handler != nullptr) {
      disconnect_legacy_handler(disconnect_legacy_handler_self, nullptr);
      connect_state = UsartConnectState::Disconnect;
    }
    if (connect_state == UsartConnectState::ConnectBaseStn2025 &&
        disconnect_basestn2025_handler != nullptr) {
      disconnect_basestn2025_handler(disconnect_basestn2025_handler_self,
                                     nullptr);
      connect_state = UsartConnectState::Disconnect;
    }
    if (connect_state == UsartConnectState::ConnectPeer2025 &&
        disconnect_peer2025_handler != nullptr) {
      disconnect_peer2025_handler(disconnect_peer2025_handler_self, nullptr);
      connect_state = UsartConnectState::Disconnect;
    }
  }
  if (next_state != connect_state && connect_state != UsartConnectState::Init) {
    if (next_state == UsartConnectState::ConnectLegacy &&
        connect_legacy_handler != nullptr) {
      connect_legacy_handler(connect_legacy_handler_self, nullptr);
    }
    if (next_state == UsartConnectState::ConnectPeer2025 &&
        connect_peer2025_handler != nullptr) {
      connect_peer2025_handler(connect_peer2025_handler_self, nullptr);
    }
    if (next_state == UsartConnectState::ConnectBaseStn2025 &&
        connect_basestn2025_handler != nullptr) {
      connect_basestn2025_handler(connect_basestn2025_handler_self, nullptr);
    }
  }
  recv_pong_flags = 0;
  connect_state = next_state;
}

void XBoardLogic::ParseRoutine(void *) { ParsePacket(); }

void XBoardLogic::PingRoutine(void *) {
  SendPing();
  CheckPing();
  CheckPong();
}

enum UsartConnectState XBoardLogic::GetConnectState() { return connect_state; }

}  // namespace xboard
}  // namespace service
}  // namespace hitcon
