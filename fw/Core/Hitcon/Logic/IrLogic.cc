#include "IrLogic.h"

#include <Logic/IrLogic.h>
#include <Service/IrService.h>

#include <cstdint>

namespace hitcon {
namespace ir {

IrLogic irLogic;

IrLogic::IrLogic() {}

void IrLogic::Init() {
  // Set callback
  irService.SetOnBufferReceived((callback_t)&IrLogic::OnBufferReceived, this);
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
  uint8_t cnt = 0;
  while (x > 0) {
    cnt++;
    x -= x & (-x);
  }
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

void IrLogic::OnBufferReceived(uint8_t *buffer) {
  // receive buffer -> check if packet_end -> call callback
  // buffer starts from LSB

  // Here is a DOS feature that if someone send a packet_header
  // then it can cause decode + receive fail
  static IrPacket packet;
  static uint8_t bit{0};
  for (size_t i = 0; i < IR_SERVICE_BUFFER_SIZE; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t is_on = (buffer[i] & (1 << j)) ? 1 : 0;
      switch (packet_state) {
        case STATE_START:
          packet_buf <<= 1;
          packet_buf |= is_on;
          packet_buf &= IR_PACKET_HEADER_MASK;
          if (packet_buf == IR_PACKET_HEADER_PACKED) {
            packet_state = STATE_SIZE;
            packet_buf = 0;
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
            packet.size_ = ((packet.size_ << 1) | decode_bit(bit));
            bit = 0;
          }
          // end of size section (decode ratio * 1 byte)
          if (packet_buf == 4 * 8) packet_state = STATE_DATA;
          break;
        case STATE_DATA:
          // here, denote packet_buf as n of consumed data bit
          packet_buf++;
          bit <<= 1;
          bit |= is_on;
          if ((packet_buf & 3) == 0) {
            if (decode_bit(bit) == BIT_INVALID) {
              // decode error
              packet_state = STATE_START;
              return;
            }
            packet.data_[packet_buf >> 2] = decode_bit(bit);
          }
          if ((packet_buf >> 2) == packet.size_) {
            // packet_end
            callback(callback_arg, reinterpret_cast<void *>(&packet));
          }
          break;
        default:
          break;
      }
    }
  }
}

void IrLogic::SetOnPacketReceived(callback_t callback, void *callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

bool IrLogic::SendPacket(uint8_t *data, size_t len) {
  IrPacket packet;
  EncodePacket(data, len, packet);
  return irService.SendBuffer(packet.data(), packet.size(), true);
}

void IrLogic::EncodePacket(uint8_t *data, size_t len, IrPacket &packet) {
  size_t i = 0;
  for (; i < PACKET_START_LEN; i++) {
    packet.data_[i] = PACKET_START[i];
  }
  for (size_t j = 0; j < PACKET_SIZE_LEN; j++) {
    packet.data_[i++] = (len >> j) & 1;  // Little-endian
  }
  int parity = 0;
  for (size_t j = 0; j < len; j++) {
    for (size_t k = 0; k < 8; k++) {
      packet.data_[i++] = (data[j] >> k) & 1;  // Little-endian
      parity ^= packet.data_[i - 1];
    }
  }
  packet.data_[i++] = parity;
  for (size_t j = 0; j < PACKET_END_LEN; j++) {
    packet.data_[i++] = PACKET_END[j];
  }
  packet.size_ = i;
}

// Return -1 if the bit at pos is invalid.
// Return 1 if the bit at pos is 1.
// Return 0 if the bit at pos is 0.
int decode_bit_at_pos(queue_t &buf, size_t pos) {
  int num_ones = 0;
  for (size_t j = 0; j < DECODE_SAMPLE_RATIO; j++) {
    num_ones +=
        buf.buf[(buf.start + pos * DECODE_SAMPLE_RATIO + j) % QUEUE_MAX_SIZE];
  }
  if (num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD) return 1;
  if (DECODE_SAMPLE_RATIO - num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD) return 0;
  return -1;
}

int IrLogic::DecodePacket(IrPacket &packet, size_t *len, uint8_t *decodedData) {
  static queue_t buf;
  int size = 0;
  int parity = 0;

  // Append data to buffer
  for (size_t i = 0; i < packet.size(); i++) {
    buf.buf[buf.end] = packet.data()[i];
    buf.end = (buf.end + 1) % QUEUE_MAX_SIZE;
  }

  // TODO: profile and optimize
  while (1) {
    // Check if buffer has enough data
    if (buf.size() < PACKET_MAX_LEN * DECODE_SAMPLE_RATIO) return 0;

    // Find start sequence
    for (size_t i = 0; i < PACKET_START_LEN; i++) {
      if (decode_bit_at_pos(buf, i) != PACKET_START[i]) goto next;
    }

    // Find size (little-endian)
    size = 0;
    for (size_t i = 0; i < PACKET_SIZE_LEN; i++) {
      int bit = decode_bit_at_pos(buf, PACKET_START_LEN + i);
      if (bit == -1) goto next;
      size |= bit << i;
    }
    if (size == 0) goto next;

    // Parse data (little-endian)
    parity = 0;
    for (int i = 0; i < size; i++) {
      uint8_t byte = 0;
      for (int j = 0; j < 8; j++) {
        int bit = decode_bit_at_pos(
            buf, PACKET_START_LEN + PACKET_SIZE_LEN + i * 8 + j);
        if (bit == -1) goto next;
        byte |= bit << j;
        parity ^= bit;
      }
      decodedData[i] = byte;
    }
    *len = size;

    // Check parity
    if (parity !=
        decode_bit_at_pos(buf, PACKET_START_LEN + PACKET_SIZE_LEN + size * 8))
      goto next;

    // Find end sequence
    for (size_t i = 0; i < PACKET_END_LEN; i++) {
      if (decode_bit_at_pos(buf, PACKET_START_LEN + PACKET_SIZE_LEN + size * 8 +
                                     PACKET_PARITY_LEN + i) != PACKET_END[i])
        goto next;
    }

    // Remove the packet from buffer
    buf.start = (buf.start + (PACKET_START_LEN + PACKET_SIZE_LEN + size * 8 +
                              PACKET_PARITY_LEN + PACKET_END_LEN) *
                                 DECODE_SAMPLE_RATIO) %
                QUEUE_MAX_SIZE;

    return 1;

  next:
    buf.start++;
  }
}

int IrLogic::GetLoadFactor() {
  // TODO
  return 10000;
}

}  // namespace ir
}  // namespace hitcon
