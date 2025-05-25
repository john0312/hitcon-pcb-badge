#include <Logic/BaseStnHub.h>
#include <Logic/IrController.h>

#include <cstring>
using namespace hitcon::service::sched;
using namespace hitcon::ir;
using namespace hitcon::logic::cdc;

namespace hitcon {
namespace basestn {

BaseStationHub g_basestn_hub;

BaseStationHub::BaseStationHub()
    : _routine_task(490, (task_callback_t)&BaseStationHub::Routine, this, 20) {}

void BaseStationHub::Init() {
  g_cdc_logic.SetOnPacketArrive((callback_t)&BaseStationHub::QueueTxHandler,
                                this, FnId::QueueStationTX);
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
}

bool BaseStationHub::WriteBuffer(BufferType buffer_type, uint8_t* data,
                                 size_t cnt) {
  if (cnt > kBufferSize - 1) {
    return false;
  }
  auto& buffer = buffer_type == BufferType::RX ? rx_buffer : tx_buffer;
  auto& queue = buffer_type == BufferType::RX ? rxq : txq;
  // remove the oldest buffer if the queue is full
  if (queue.IsFull()) {
    auto idx = queue.Front();
    buffer[idx * kBufferSize] = 0;
    queue.PopFront();
  }
  for (uint8_t i = 0; i < kBufferCount; i++) {
    auto& status = reinterpret_cast<BufferMeta&>(buffer[i * kBufferSize]);
    if (status.locked) continue;
    status.locked = 1;
    status.length = cnt;
    memcpy(&buffer[i * kBufferSize + 1], data, cnt);
    queue.PushBack(i);
    return true;
  }
  return false;
}

bool BaseStationHub::ReadBuffer(BufferType buffer_type, uint8_t* data,
                                size_t& cnt) {
  auto& buffer = buffer_type == BufferType::RX ? rx_buffer : tx_buffer;
  auto& queue = buffer_type == BufferType::RX ? rxq : txq;
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
    return false;
  }
  memcpy(data, &buffer[idx * kBufferSize + 1], status.length);
  cnt = status.length;
  status.locked = 0;
  queue.PopFront();
  return true;
}

void BaseStationHub::OnIrPacketRecv(uint8_t* data, size_t cnt) {
  WriteBuffer(BufferType::RX, data, cnt);
}

void BaseStationHub::QueueTxHandler(PacketCallbackArg* arg) {
  bool success = WriteBuffer(BufferType::TX, arg->data, arg->len);
  uint8_t buffer[HEADER_SZ + 1] = {0};
  auto header = reinterpret_cast<PktHdr*>(buffer);
  auto payload = buffer + HEADER_SZ;
  header->id = arg->id;
  header->type = 0x81;
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
    return;
  }
  auto irdata = &buffer[idx * kBufferSize + 1];
  bool success = irLogic.SendPacket(irdata, status.length);
  // release if sent successfully
  if (success) {
    queue.PopFront();
    status.locked = 0;
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
    return;
  }
  uint8_t cdc_pkt[HEADER_SZ + kBufferSize] = {0};
  auto cdc_hdr = reinterpret_cast<PktHdr*>(cdc_pkt);
  cdc_hdr->type = 5;  // PopRxBufferRequest
  cdc_hdr->len = status.length;
  memcpy(cdc_pkt + HEADER_SZ, &buffer[idx * kBufferSize + 1], status.length);
  g_cdc_logic.SetSeq(cdc_hdr);
  bool sent = g_cdc_logic.SendPacket(cdc_pkt);
  // release if sent successfully
  if (sent) {
    status.locked = 0;
    queue.PopFront();
  }
}

void BaseStationHub::Routine(void*) {
  SendToIr();
  SendToBaseStation();
}

}  // namespace basestn
}  // namespace hitcon
