#include <Logic/BaseStnHub.h>
#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>

#include <cstring>
using namespace hitcon::service::sched;
using namespace hitcon::ir;
using namespace hitcon::logic::cdc;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::RecvFnId;

namespace hitcon {
namespace basestn {

BaseStationHub g_basestn_hub;

BaseStationHub::BaseStationHub()
    : _routine_task(490, (task_callback_t)&BaseStationHub::Routine, this, 20) {}

void BaseStationHub::Init() {
  g_cdc_logic.SetOnPacketArrive((callback_t)&BaseStationHub::QueueTxHandler,
                                this, FnId::QueueStationTX);
  g_cdc_logic.SetOnPacketArrive((callback_t)&BaseStationHub::QueueXbTxHandler,
                                this, FnId::QueueXBoardTX);
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  g_xboard_logic.SetOnPacketArrive(
      (callback_t)&BaseStationHub::OnXBoardPacketRecv, this,
      RecvFnId::IR_TO_BASE_STATION);
}

bool BaseStationHub::WriteBuffer(BufferType buffer_type, uint8_t* data,
                                 size_t cnt) {
  if (cnt > kBufferSize - 1) {
    return false;
  }
  auto& buffer = *buffer_map[static_cast<uint8_t>(buffer_type)];
  auto& queue = *queue_map[static_cast<uint8_t>(buffer_type)];
  // remove the oldest buffer if the queue is full
  if (queue.IsFull()) {
    auto idx = queue.Front();
    buffer[idx * kBufferSize] = 0;
    queue.PopFront();
    metric_map[static_cast<uint8_t>(buffer_type)]->dropped++;
  }
  for (uint8_t i = 0; i < kBufferCount; i++) {
    auto& status = reinterpret_cast<BufferMeta&>(buffer[i * kBufferSize]);
    if (status.locked) continue;
    status.locked = 1;
    status.length = cnt;
    memcpy(&buffer[i * kBufferSize + 1], data, cnt);
    queue.PushBack(i);
    metric_map[static_cast<uint8_t>(buffer_type)]->put++;
    return true;
  }
  return false;
}

bool BaseStationHub::ReadBuffer(BufferType buffer_type, uint8_t* data,
                                size_t& cnt) {
  auto& buffer = *buffer_map[static_cast<uint8_t>(buffer_type)];
  auto& queue = *queue_map[static_cast<uint8_t>(buffer_type)];
  if (queue.IsEmpty()) {
    cnt = 0;
    return false;
  }
  auto idx = queue.Front();
  auto& status = reinterpret_cast<BufferMeta&>(buffer[idx * kBufferSize]);
  if (!status.locked || status.length > cnt) {
    cnt = 0;
    status.locked = 0;
    queue.PopFront();
    metric_map[static_cast<uint8_t>(buffer_type)]->dropped++;
    return false;
  }
  memcpy(data, &buffer[idx * kBufferSize + 1], status.length);
  cnt = status.length;
  status.locked = 0;
  queue.PopFront();
  metric_map[static_cast<uint8_t>(buffer_type)]->taken++;
  return true;
}

void BaseStationHub::OnIrPacketRecv(uint8_t* data, size_t cnt) {
  WriteBuffer(BufferType::RX, data, cnt);
}

void BaseStationHub::OnXBoardPacketRecv(void* arg1) {
  auto* packet =
      reinterpret_cast<hitcon::service::xboard::PacketCallbackArg*>(arg1);
  WriteBuffer(BufferType::XBRX, packet->data, packet->len);
}

const BufferMetric& BaseStationHub::GetBufferMetric(BufferType buffer_type) {
  return *metric_map[static_cast<uint8_t>(buffer_type)];
}

// private methods

void BaseStationHub::QueueTxHandler(PacketCallbackArg* arg) {
  bool success = WriteBuffer(BufferType::TX, arg->data, arg->len);
  // respond ack to control plane
  uint8_t buffer[HEADER_SZ + 1] = {0};
  auto header = reinterpret_cast<PktHdr*>(buffer);
  auto payload = buffer + HEADER_SZ;
  header->id = arg->id;
  header->type = 0x81;
  header->len = 1;
  payload[0] = success ? 2 : 1;
  g_cdc_logic.SendPacket(buffer);
}

void BaseStationHub::QueueXbTxHandler(PacketCallbackArg* arg) {
  bool success = WriteBuffer(BufferType::XBTX, arg->data, arg->len);
  // respond ack to control plane
  uint8_t buffer[HEADER_SZ + 1] = {0};
  auto header = reinterpret_cast<PktHdr*>(buffer);
  auto payload = buffer + HEADER_SZ;
  header->id = arg->id;
  header->type = 0x91;
  header->len = 1;
  payload[0] = success ? 2 : 1;
  g_cdc_logic.SendPacket(buffer);
}

void BaseStationHub::SendToIr() {
  // try to send ir
  auto& buffer = tx_buffer;
  auto& queue = txq;
  if (queue.IsEmpty()) return;
  auto idx = queue.Front();
  auto& status = reinterpret_cast<BufferMeta&>(buffer[idx * kBufferSize]);
  if (!status.locked) {
    // already released
    queue.PopFront();
    tx_metric.dropped++;
    return;
  }
  auto irdata = &buffer[idx * kBufferSize + 1];
  bool success = irLogic.SendPacket(irdata, status.length);
  // release if sent successfully
  if (success) {
    queue.PopFront();
    status.locked = 0;
    tx_metric.taken++;
  }
}

void BaseStationHub::SendToXBoard() {
  // try to send to xboard
  auto& buffer = xbtx_buffer;
  auto& queue = xbtxq;
  if (queue.IsEmpty()) return;
  auto idx = queue.Front();
  auto& status = reinterpret_cast<BufferMeta&>(buffer[idx * kBufferSize]);
  if (!status.locked) {
    // already released
    queue.PopFront();
    xbtx_metric.dropped++;
    return;
  }
  auto xdata = &buffer[idx * kBufferSize + 1];
  if (g_xboard_logic.GetConnectState() == hitcon::service::xboard::Connect) {
    g_xboard_logic.QueueDataForTx(xdata, status.length,
                                  RecvFnId::IR_TO_ATTENDEE);
    queue.PopFront();
    status.locked = 0;
    xbtx_metric.taken++;
  }
}

void BaseStationHub::SendToBaseStation() {
  // try to send to base station
  auto& buffer = rx_buffer;
  auto& queue = rxq;
  if (queue.IsEmpty()) return;
  auto idx = queue.Front();
  auto& status = reinterpret_cast<BufferMeta&>(buffer[idx * kBufferSize]);
  if (!status.locked) {
    // already released
    queue.PopFront();
    rx_metric.dropped++;
    return;
  }
  uint8_t cdc_pkt[HEADER_SZ + kBufferSize] = {0};
  auto cdc_hdr = reinterpret_cast<PktHdr*>(cdc_pkt);
  cdc_hdr->type = PopBaseRxBuffer;  // PopRxBufferRequest
  cdc_hdr->len = status.length;
  memcpy(cdc_pkt + HEADER_SZ, &buffer[idx * kBufferSize + 1], status.length);
  g_cdc_logic.SetSeq(cdc_hdr);
  bool sent = g_cdc_logic.SendPacket(cdc_pkt);
  // release if sent successfully
  if (sent) {
    status.locked = 0;
    queue.PopFront();
    rx_metric.taken++;
  }
}

void BaseStationHub::SendXbToBaseStation() {
  // try to send to base station
  auto& buffer = xbrx_buffer;
  auto& queue = xbrxq;
  if (queue.IsEmpty()) return;
  auto idx = queue.Front();
  auto& status = reinterpret_cast<BufferMeta&>(buffer[idx * kBufferSize]);
  if (!status.locked) {
    // already released
    queue.PopFront();
    xbrx_metric.dropped++;
    return;
  }
  uint8_t cdc_pkt[HEADER_SZ + kBufferSize] = {0};
  auto cdc_hdr = reinterpret_cast<PktHdr*>(cdc_pkt);
  cdc_hdr->type = PopBaseXbRxBuffer;  // PopRxBufferRequest
  cdc_hdr->len = status.length;
  memcpy(cdc_pkt + HEADER_SZ, &buffer[idx * kBufferSize + 1], status.length);
  g_cdc_logic.SetSeq(cdc_hdr);
  bool sent = g_cdc_logic.SendPacket(cdc_pkt);
  // release if sent successfully
  if (sent) {
    status.locked = 0;
    queue.PopFront();
    xbrx_metric.taken++;
  }
}

void BaseStationHub::OnXBoardConnect() {
  // Clear XBoard metrics when XBoard connects
  xbrx_metric = {0, 0, 0};
  xbtx_metric = {0, 0, 0};
}

void BaseStationHub::Routine(void*) {
  // to control plane
  SendToBaseStation();
  SendXbToBaseStation();

  // to peripheral
  SendToIr();
  SendToXBoard();
}

}  // namespace basestn
}  // namespace hitcon
