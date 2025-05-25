#include "CdcLogic.h"

#include <algorithm>
#include <cstring>

#include "Service/CdcService.h"

using namespace hitcon::service::cdc;
using hitcon::service::sched::scheduler;
using hitcon::service::sched::task_callback_t;

namespace hitcon {
namespace logic {
namespace cdc {

CdcLogic g_cdc_logic;

CdcLogic::CdcLogic()
    : _parse_routine(490, (task_callback_t)&CdcLogic::ParseRoutine, this, 20) {}

void CdcLogic::Init() {
  scheduler.Queue(&_parse_routine, nullptr);
  scheduler.EnablePeriodic(&_parse_routine);
  g_cdc_service.SetOnDataReceived((on_rx_callback_t)&CdcLogic::OnDataReceived,
                                  this);
}

bool CdcLogic::SendPacket(uint8_t* data) {
  auto header = reinterpret_cast<PktHdr*>(data);
  header->preamble = PREAMBLE;
  return g_cdc_service.Send(data, HEADER_SZ + header->len);
}

void CdcLogic::SetSeq(PktHdr* pkt_hdr) {
  // set auto increment sequence number
  pkt_hdr->id = auto_seq++;
}

void CdcLogic::SetOnPacketArrive(callback_t callback, void* self,
                                 FnId handler_id) {
  packet_arrive_cbs[handler_id] = {callback, self};
}

// private
uint16_t inc_head(uint16_t head, size_t offset) {
  return (head + offset) % BUF_CAPACITY;
}

bool CdcLogic::IsBufferFull() { return inc_head(prod_head, 1) == cons_head; }

uint16_t CdcLogic::BufferLen() {
  if (prod_head < cons_head) {
    return BUF_CAPACITY - cons_head + prod_head;
  } else {
    return prod_head - cons_head;
  }
}

uint16_t CdcLogic::BufferSpace() { return BUF_CAPACITY - BufferLen() - 1; }

void CdcLogic::OnDataReceived(uint8_t* data, size_t len) {
  // write as many bytes as possible
  len = std::min(len, (size_t)BufferSpace());
  uint16_t next_prod_head = inc_head(prod_head, len);
  if (next_prod_head == cons_head) {
    return;
  }
  if (next_prod_head < prod_head) {
    size_t sz1 = BUF_CAPACITY - prod_head;
    memcpy(buf + prod_head, data, sz1);
    memcpy(buf, data + sz1, len - sz1);
  } else {
    memcpy(buf + prod_head, data, len);
  }
  prod_head = next_prod_head;
}

bool CdcLogic::TryReadBytes(uint8_t* dst, size_t size, uint16_t head_offset) {
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
      (_cons_head > prod_head ? 0 : BUF_CAPACITY) - prod_head + _cons_head;
  if (remain_size < size) {
    return false;
  }
  uint16_t next_cons_head = inc_head(_cons_head, size);
  if (next_cons_head < _cons_head) {
    uint16_t sz1 = BUF_CAPACITY - _cons_head;
    memcpy(dst, buf + _cons_head, sz1);
    memcpy(dst + sz1, buf, size - sz1);
  } else {
    memcpy(dst, buf + _cons_head, size);
  }
  return true;
}

void CdcLogic::ParsePacket() {
  size_t bytes_processed = 0;
  while (cons_head != prod_head && bytes_processed < 16) {
    if (buf[cons_head] != 0x55) {
      cons_head = inc_head(cons_head, 1);
      ++bytes_processed;
      continue;
    }
    uint16_t in_buf_size =
        (prod_head > cons_head ? 0 : BUF_CAPACITY) + prod_head - cons_head;
    if (in_buf_size < HEADER_SZ) {
      break;
    }

    uint8_t pkt[HEADER_SZ + PKT_PAYLOAD_LEN_MAX] = {0};
    PktHdr* header = reinterpret_cast<PktHdr*>(pkt);
    uint8_t* payload = pkt + HEADER_SZ;
    TryReadBytes(reinterpret_cast<uint8_t*>(header), HEADER_SZ);
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

    cons_head = inc_head(cons_head, HEADER_SZ + header->len);

    // app callbacks
    if (header->type < FnId::MAX) {
      PacketCallbackArg packet_cb_arg{
          .id = header->id,
          .data = payload,
          .len = header->len,
      };
      auto [recv_fn, recv_self] = packet_arrive_cbs[header->type];
      if (recv_fn != nullptr) recv_fn(recv_self, &packet_cb_arg);
    }
    // handle at most one packet each time
    break;
  }
}

void CdcLogic::ParseRoutine(void*) { ParsePacket(); }

}  // namespace cdc
}  // namespace logic
}  // namespace hitcon
