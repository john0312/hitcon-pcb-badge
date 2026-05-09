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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
XBoardLogic::XBoardLogic()
    : _parse_routine(490, (task_callback_t)&XBoardLogic::ParseRoutine, this,
                     20),
      _ping_routine(490, (task_callback_t)&XBoardLogic::PingRoutine, this,
                    200) {}
#pragma GCC diagnostic pop

void XBoardLogic::Init() {
  scheduler.Queue(&_parse_routine, nullptr);
  scheduler.EnablePeriodic(&_parse_routine);
  scheduler.Queue(&_ping_routine, nullptr);
  scheduler.EnablePeriodic(&_ping_routine);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_xboard_service.SetOnByteRx((callback_t)&XBoardLogic::OnByteArrive, this);
#pragma GCC diagnostic pop
}

void XBoardLogic::QueueDataForTx(const uint8_t *packet, uint8_t packet_len,
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

void XBoardLogic::SetOnConnect(PeerType peer, callback_t callback, void *self) {
  my_assert(peer != PeerType::None && peer != PeerType::NUM_PEER_TYPES);
  connect_cbs[static_cast<size_t>(peer)] = {callback, self};
}

void XBoardLogic::SetOnDisconnect(PeerType peer, callback_t callback,
                                  void *self) {
  my_assert(peer != PeerType::None && peer != PeerType::NUM_PEER_TYPES);
  disconnect_cbs[static_cast<size_t>(peer)] = {callback, self};
}

void XBoardLogic::SetOnPacketArrive(callback_t callback, void *self,
                                    RecvFnId handler_id) {
  packet_arrive_cbs[handler_id] = {callback, self};
}

// private functions

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
  *reinterpret_cast<Frame *>(pkt) = Frame{PREAMBLE, 0, 0, SELF_PONG_TYPE, 0};
  reinterpret_cast<Frame *>(pkt)->checksum = fast_crc32(pkt, HEADER_SZ);
  // for (int i = 0; i < sizeof(Frame); i++) {
  //   pkt[i] = (0x11+i)&0x0FF;
  //   pkt[i] = 0;
  // }
  g_xboard_service.QueueDataForTx(pkt, sizeof(pkt));
}

void XBoardLogic::OnByteArrive(void *arg1) {
  uint8_t b = static_cast<uint8_t>(reinterpret_cast<size_t>(arg1));
  if (rx_queue.IsFull()) {
    // drop the data
    AssertOverflow();
    return;
  }
  rx_queue.PushBack(b);
}

void XBoardLogic::ParsePacket() {
  size_t bytes_processed = 0;
  while (!rx_queue.IsEmpty() && bytes_processed < 16) {
    if (rx_queue.Front() != 0x55) {
      rx_queue.PopFront();
      ++bytes_processed;
      continue;
    }
    if (rx_queue.Size() < HEADER_SZ) {
      break;
    }

    uint8_t pkt[HEADER_SZ + PKT_PAYLOAD_LEN_MAX] = {0};
    Frame *header = reinterpret_cast<Frame *>(pkt);
    uint8_t *payload = pkt + HEADER_SZ;
    rx_queue.PeekSegment(reinterpret_cast<uint8_t *>(header), HEADER_SZ, 0);
    if (header->preamble != PREAMBLE) {
      rx_queue.PopFront();
      ++bytes_processed;
      continue;
    }
    if (header->len >= PKT_PAYLOAD_LEN_MAX) {
      // invalid packet, skip this packet (preamble 8 bytes)
      rx_queue.RemoveFrontMulti(8);

      bytes_processed += 8;
      continue;
    }
    if (!rx_queue.PeekSegment(payload, header->len, HEADER_SZ)) {
      // no enough bytes to read, wait more bytes in
      return;
    }

    uint32_t recv_check = header->checksum;
    header->checksum = 0;
    if (fast_crc32(pkt, HEADER_SZ + header->len +
                            PADDING_MAP[header->len & 0b11]) != recv_check) {
      rx_queue.RemoveFrontMulti(8);
      bytes_processed += 8;
      continue;
    }

    // pass checking, valid packet now
    rx_queue.RemoveFrontMulti(HEADER_SZ + header->len);
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
  // 1. Decode received pong.
  PeerType received_peer = PeerType::None;
  if (recv_pong_flags == 0x01) {
    received_peer = PeerType::Legacy;
  } else if (recv_pong_flags == 0x02) {
    received_peer = PeerType::Peer2025;
  } else if (recv_pong_flags == 0x04) {
    received_peer = PeerType::BaseStn2025;
  }

  // 2. A pong is valid only if it keeps the current peer or connects from None.
  // Pong from a different peer than the current one is treated as no pong, so
  // the state machine only transitions via None.
  bool valid_pong = (received_peer != PeerType::None) &&
                    (peer == PeerType::None || peer == received_peer);

  // 3. Compute next_peer.
  PeerType next_peer;
  if (valid_pong) {
    next_peer = received_peer;
    no_pong_count = 0;
  } else {
    if (no_pong_count < 3) ++no_pong_count;
    next_peer = (no_pong_count >= 3) ? PeerType::None : peer;
  }

  // 4. Dispatch transition. Invariant guarantees one side is None.
  if (next_peer != peer) {
    if (peer == PeerType::None) {
      // None -> X: pure connect.
      auto [cb, self] = connect_cbs[static_cast<size_t>(next_peer)];
      if (cb != nullptr) cb(self, nullptr);
      g_suspender.IncBlocker();
    } else {
      // X -> None: pure disconnect.
      auto [cb, self] = disconnect_cbs[static_cast<size_t>(peer)];
      if (cb != nullptr) cb(self, nullptr);
      g_suspender.DecBlocker();
    }
    peer = next_peer;
  }
  recv_pong_flags = 0;
}

void XBoardLogic::ParseRoutine(void *) { ParsePacket(); }

void XBoardLogic::PingRoutine(void *) {
  SendPing();
  CheckPing();
  CheckPong();
}

}  // namespace xboard
}  // namespace service
}  // namespace hitcon
