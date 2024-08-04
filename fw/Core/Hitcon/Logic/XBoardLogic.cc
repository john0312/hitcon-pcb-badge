#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

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
}  // namespace

struct Frame {
  uint64_t preamble;  // 0xD555555555555555
  uint16_t id;
  uint8_t len;
  uint8_t type;  // 208(0xd0): ping
  uint16_t checksum;
};
constexpr size_t HEADER_SZ = sizeof(Frame);

void XBoardLogic::SendPing() {
  uint8_t pkt[HEADER_SZ] = {0};
  *(Frame *)pkt = Frame{0xD555555555555555, 0, 0, PING_TYPE, 0};
  // for (int i = 0; i < sizeof(Frame); i++) {
  //   pkt[i] = (0x11+i)&0x0FF;
  //   pkt[i] = 0;
  // }
  g_xboard_service.QueueDataForTx(pkt, sizeof(pkt));
}

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

void XBoardLogic::ParsePacket() {
  recv_ping = false;
  while (cons_head != prod_head) {
    if (rx_buf[cons_head] != 0x55) {
      cons_head = inc_head(cons_head, 1);
      continue;
    }
    uint16_t in_buf_size =
        (prod_head > cons_head ? 0 : RX_BUF_SZ) + prod_head - cons_head;
    if (in_buf_size < sizeof(Frame)) {
      break;
    }

    Frame header;
    TryReadBytes((uint8_t *)&header, HEADER_SZ);
    if (header.preamble != 0xD555555555555555) {
      cons_head = inc_head(cons_head, 1);
      continue;
    }
    if (!TryReadBytes((uint8_t *)&packet_payload, header.len, sizeof(Frame))) {
      // no enough bytes to read, wait more bytes in
      return;
    }
    cons_head = inc_head(cons_head, HEADER_SZ + header.len);
    if (header.type == PING_TYPE) {
      recv_ping = true;
      continue;
    }
    if (header.type < RecvFnId::MAX) {
      PacketCallbackArg packet_cb_arg;
      memcpy(packet_cb_arg.data, &packet_payload, header.len);
      packet_cb_arg.len = header.len;
      auto [recv_fn, recv_self] = packet_arrive_cbs[header.type];
      if (recv_fn != nullptr) recv_fn(recv_self, &packet_cb_arg);
    }
  }
}

XBoardLogic::XBoardLogic()
    : _routine_task(490, (task_callback_t)&XBoardLogic::Routine, this, 200) {}

void XBoardLogic::Init() {
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  g_xboard_service.SetOnByteRx((callback_t)&XBoardLogic::OnByteArrive, this);
}

void XBoardLogic::QueueDataForTx(uint8_t *packet, size_t packet_len,
                                 RecvFnId handler_id) {
  uint8_t pkt[64];
  *(Frame *)pkt =
      Frame{0xD555555555555555, 0, (uint8_t)packet_len, (uint8_t)handler_id, 0};
  for (int i = 0; i < packet_len; ++i) {
    pkt[i + HEADER_SZ] = packet[i];
  }
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

void XBoardLogic::Routine(void *) {
  SendPing();
  ParsePacket();
  CheckPing();
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

void XBoardLogic::CheckPing() {
  if (!recv_ping) {
    if (no_ping_count < 3) {
      ++no_ping_count;
    }
  } else {
    no_ping_count = 0;
  }
  UsartConnectState next_state = no_ping_count >= 3 ? Disconnect : Connect;
  if (next_state != connect_state) {
    if (next_state == Disconnect && disconnect_handler != nullptr) {
      disconnect_handler(disconnect_handler_self, nullptr);
    } else if (next_state == Connect && connect_handler != nullptr) {
      connect_handler(connect_handler_self, nullptr);
    }
  }
  connect_state = next_state;
}

}  // namespace xboard
}  // namespace service
}  // namespace hitcon
