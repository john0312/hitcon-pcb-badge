#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Logic/crc32.h>
#include <Service/Suspender.h>

#include <cstring>

#include "XBoardLogicCbs.h"

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;

namespace hitcon {
namespace service {
namespace xboard {

XBoardLogic g_xboard_logic;

namespace {
constexpr uint8_t PADDING_MAP[] = {0, 3, 2, 1};
}  // namespace

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
  *(Frame *)pkt = Frame{0xD555555555555555, 0, packet_len,
                        static_cast<uint8_t>(handler_id), 0};
  for (uint8_t i = 0; i < packet_len; ++i) {
    pkt[i + HEADER_SZ] = packet[i];
  }
  reinterpret_cast<Frame *>(pkt)->checksum =
      fast_crc32(pkt, HEADER_SZ + packet_len + PADDING_MAP[packet_len & 0b11]);
  g_xboard_service.QueueDataForTx(pkt, HEADER_SZ + packet_len);
}

void XBoardLogic::SetOnConnect(callback_t callback, void *self) {
  connect_handler = callback;
  connect_handler_self = self;
}

void XBoardLogic::SetOnDisconnect(callback_t callback, void *self) {
  disconnect_handler = callback;
  disconnect_handler_self = self;
}

void XBoardLogic::SetOnPacketArrive(callback_t callback, void *self,
                                    RecvFnId handler_id) {
  packet_arrive_cbs[handler_id] = {callback, self};
}

// private functions

void XBoardLogic::SendPing() {
  uint8_t pkt[HEADER_SZ] = {0};
  *reinterpret_cast<Frame *>(pkt) =
      Frame{0xD555555555555555, 0, 0, PING_TYPE, 0};
  reinterpret_cast<Frame *>(pkt)->checksum = fast_crc32(pkt, HEADER_SZ);
  // for (int i = 0; i < sizeof(Frame); i++) {
  //   pkt[i] = (0x11+i)&0x0FF;
  //   pkt[i] = 0;
  // }
  g_xboard_service.QueueDataForTx(pkt, sizeof(pkt));
}

void XBoardLogic::SendPong() {
  uint8_t pkt[HEADER_SZ] = {0};
  *reinterpret_cast<Frame *>(pkt) =
      Frame{0xD555555555555555, 0, 0, SELF_PONG_TYPE, 0};
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
    if (header->preamble != 0xD555555555555555) {
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
    if (header->type == PONG_TYPE || header->type == PONG_PEER2025_TYPE ||
        header->type == PONG_BASESTN2025_TYPE) {
      last_pong = header->type;
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
    SendPong();
  }
  recv_ping = false;
}

void XBoardLogic::CheckPong() {
  if (connect_state == UsartConnectState::Init) no_pong_count = 3;
  if (last_pong == 0) {
    if (no_pong_count < 3) {
      ++no_pong_count;
    }
  } else {
    // last pong == current peer
    if (last_pong == peer) {
      no_pong_count = 0;
    } else {
      // not equal to current peer
      if (no_pong_count < 3) {
        ++no_pong_count;
      }
    }
  }
  if (no_pong_count >= 3) {
    if (last_pong != 0) {
      peer = static_cast<PeerType>(last_pong);
      no_pong_count = 0;
    } else {
      peer = PeerType::None;
    }
  }
  UsartConnectState next_state = no_pong_count >= 3 ? Disconnect : Connect;
  if (next_state != connect_state) {
    if (next_state == Disconnect && connect_state != UsartConnectState::Init) {
      g_suspender.DecBlocker();
    } else if (next_state == Connect) {
      g_suspender.IncBlocker();
    }
  }

  if (next_state != connect_state && connect_state != UsartConnectState::Init) {
    if (next_state == Disconnect && disconnect_handler != nullptr) {
      disconnect_handler(disconnect_handler_self, nullptr);
      OnDisconnectHandler();
    } else if (next_state == Connect && connect_handler != nullptr) {
      connect_handler(connect_handler_self, nullptr);
      OnConnectHandler();
    }
  }
  last_pong = 0;
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
