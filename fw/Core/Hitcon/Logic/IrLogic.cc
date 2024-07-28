#include "IrLogic.h"

#include <Logic/IrLogic.h>
#include <Service/IrService.h>

#include <cstdint>
#include <cstring>

using hitcon::service::sched::my_assert;

namespace hitcon {
namespace ir {

IrLogic irLogic;
service::sched::Task OnBufferReceivedTask(
    490, (service::sched::task_callback_t)&IrLogic::OnBufferReceived, &irLogic);

IrLogic::IrLogic()
    : lf_total_period(0), lf_nonzero_period(0), lowpass_loadfactor(0) {}

void IrLogic::Init() {
  // Set callback
  irService.SetOnBufferReceived(
      (callback_t)&IrLogic::OnBufferReceivedEnqueueTask, this);
}

size_t packet_buf{0};
uint8_t packet_state{0};

enum PACKET_STATE {
  STATE_START = 0,
  STATE_SIZE = 1,
  STATE_DATA = 2,
};

enum BIT_STATE {
  BIT_OFF = 0,
  BIT_ON = 1,
  BIT_INVALID = 2,
};

static uint8_t decode_bit(uint8_t x) {
  uint8_t cnt = __builtin_popcount(x & 0b1111);
  switch (cnt) {
    case 0:
    case 1:
      return BIT_OFF;
    case 3:
    case 4:
      return BIT_ON;
    case 2:
    default:
      return BIT_INVALID;
  }
}

void IrLogic::OnBufferReceivedEnqueueTask(uint8_t *buffer) {
  buffer_received_ctr = 0;
  service::sched::scheduler.Queue(&OnBufferReceivedTask, buffer);

  static_assert(IR_SERVICE_RX_ON_BUFFER_SIZE % IR_LOADFACTOR_PERIOD == 0);
  static_assert(IR_LOADFACTOR_PERIOD % sizeof(uint32_t) == 0);
  size_t pos = 0;
  uint32_t current_period = 0;
  uint32_t *u32_buffer = reinterpret_cast<uint32_t *>(buffer);
  for (int i = 0; i < IR_SERVICE_RX_ON_BUFFER_SIZE / IR_LOADFACTOR_PERIOD;
       i++) {
    current_period = 0;
    for (int j = 0; j < IR_LOADFACTOR_PERIOD / sizeof(uint32_t); j++) {
      current_period |= u32_buffer[pos];
      pos++;
    }
    if (current_period) {
      lf_nonzero_period++;
    }
    lf_total_period++;
  }
  if (lf_total_period >= IR_LOADFACTOR_SAMPLING_COUNT) {
    // current_lf is in Q15.16 fixed point.
    uint32_t current_lf = (lf_nonzero_period << 16) / lf_total_period;
    // Apply a low pass filter.
    lowpass_loadfactor =
        ((LF_ALPHA_COMPL * lowpass_loadfactor) + (LF_ALPHA * current_lf)) >> 10;
    // Reset the counters.
    lf_nonzero_period = 0;
    lf_total_period = 0;
  }
}

void IrLogic::OnBufferReceived(uint8_t *buffer) {
  // receive buffer -> check if packet_end -> call callback
  // buffer starts from LSB

  // Here is a DOS feature that if someone send a packet_header
  // then it can cause decode + receive fail
  static uint8_t bit{0};
  for (size_t i = 0; i < IR_SERVICE_RX_BUFFER_PER_RUN &&
                     buffer_received_ctr < IR_SERVICE_RX_ON_BUFFER_SIZE;
       i++, buffer_received_ctr++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t is_on = (buffer[buffer_received_ctr] & (1 << j)) ? 1 : 0;
      switch (packet_state) {
        case STATE_START:
          packet_buf <<= 1;
          packet_buf |= is_on;
          if ((packet_buf & IR_PACKET_HEADER_MASK) ==
              (IR_PACKET_HEADER_PACKED & IR_PACKET_HEADER_MASK)) {
            packet_state = STATE_SIZE;
            rx_packet.size_ = 0;
            packet_buf = 0;
            bit = 0;
          }
          break;
        case STATE_SIZE:
          // here, denote packet_buf as n of consumed size bit
          packet_buf++;
          bit <<= 1;
          bit |= is_on;
          if ((packet_buf & 3) == 0) {
            if (decode_bit(bit) == BIT_INVALID) {
              // decode error
              packet_state = STATE_START;
              return;
            }
            const uint8_t bitpos = (packet_buf / DECODE_SAMPLE_RATIO - 1);
            rx_packet.size_ = (rx_packet.size_) | (decode_bit(bit) << bitpos);
            bit = 0;
          }
          // end of size section (decode ratio * 1 byte)
          if (packet_buf == DECODE_SAMPLE_RATIO * 8) {
            if (rx_packet.size_ >= MAX_PACKET_PAYLOAD_BYTES) {
              // Packet too large.
              packet_state = STATE_START;
            } else {
              packet_state = STATE_DATA;
              packet_buf = 0;
            }
          }
          break;
        case STATE_DATA:
          // here, denote packet_buf as n of consumed data bit
          packet_buf++;
          bit <<= 1;
          bit |= is_on;
          if ((packet_buf % DECODE_SAMPLE_RATIO) == 0) {
            if (decode_bit(bit) == BIT_INVALID) {
              // decode error
              packet_state = STATE_START;
              return;
            }
            const uint8_t pos = (packet_buf / DECODE_SAMPLE_RATIO - 1) / 8;
            const uint8_t bitpos = (packet_buf / DECODE_SAMPLE_RATIO - 1) % 8;
            rx_packet.data_[pos] |= decode_bit(bit) << bitpos;
            if (pos == rx_packet.size_) {
              // packet_end
              // double buffering
              rx_packet_ctrler = rx_packet;
              callback(callback_arg,
                       reinterpret_cast<void *>(&rx_packet_ctrler));
            }
          }
          break;
        default:
          break;
      }
    }
  }
  if (buffer_received_ctr < IR_SERVICE_RX_ON_BUFFER_SIZE) {
    service::sched::scheduler.Queue(&OnBufferReceivedTask, buffer);
  }
}

void IrLogic::SetOnPacketReceived(callback_t callback, void *callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

void IrLogic::EncodePacket(uint8_t *data, size_t len, IrPacket &packet) {
  // size included
  packet.data_[0] = static_cast<uint8_t>(len);
  memcpy(packet.data_ + 1, data, len * sizeof(data[0]));
  packet.size_ = len + 1;
}

bool IrLogic::SendPacket(uint8_t *data, size_t len) {
  if (len >= MAX_PACKET_PAYLOAD_BYTES) {
    // Packet too large.
    my_assert(0);
    return false;
  }
  // TODO: Check if tx_packet is in use.
  EncodePacket(data, len, tx_packet);
  return irService.SendBuffer(tx_packet.data_, tx_packet.size_, true);
}

int IrLogic::GetLoadFactor() {
  int ret = lowpass_loadfactor;
  ret = ret * 100 * LF_MAX_SCALE;
  ret = ret >> 16;
  if (ret > 100) ret = 100;
  return ret;
}

}  // namespace ir
}  // namespace hitcon
