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

void IrLogic::OnBufferReceived(uint8_t *buffer) {
  // receive buffer -> check if packet_end -> call callback
  // buffer starts from LSB

  // Here is a DOS feature that if someone send a packet_header
  // then it can cause decode + receive fail
  static uint8_t bit{0};
  for (size_t i = 0; i < IR_SERVICE_RX_ON_BUFFER_SIZE; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t is_on = (buffer[i] & (1 << j)) ? 1 : 0;
      switch (packet_state) {
        case STATE_START:
          packet_buf <<= 1;
          packet_buf |= is_on;
          if ((packet_buf & IR_PACKET_HEADER_MASK) ==
              (IR_PACKET_HEADER_PACKED & IR_PACKET_HEADER_MASK)) {
            packet_state = STATE_SIZE;
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
            packet.size_ = ((packet.size_ << 1) | decode_bit(bit));
            bit = 0;
          }
          // end of size section (decode ratio * 1 byte)
          if (packet_buf == DECODE_SAMPLE_RATIO * 8) {
            packet_state = STATE_DATA;
            packet_buf = 0;
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
            packet.data_[pos] |= decode_bit(bit) << bitpos;
            if (packet_buf == (packet.size_ + 1) * DECODE_SAMPLE_RATIO) {
              // packet_end
              callback(callback_arg, reinterpret_cast<void *>(&packet));
            }
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

int IrLogic::GetLoadFactor() {
  // TODO
  return 10000;
}

}  // namespace ir
}  // namespace hitcon
