#ifndef HITCON_SERVICE_IR_PARAMS_H_
#define HITCON_SERVICE_IR_PARAMS_H_

#include <stddef.h>

namespace hitcon {
namespace ir {

constexpr uint8_t PACKET_START[] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 };
constexpr size_t PACKET_START_LEN = (sizeof(PACKET_START) / sizeof(uint8_t));
constexpr size_t PACKET_SIZE_LEN = 5;
constexpr size_t PACKET_DATA_MAX_BYTE = 32;
constexpr size_t PACKET_DATA_MAX_BIT = PACKET_DATA_MAX_BYTE * 8;
constexpr size_t PACKET_PARITY_LEN = 1;
constexpr uint8_t PACKET_END[] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
constexpr size_t PACKET_END_LEN = sizeof(PACKET_END) / sizeof(uint8_t);
constexpr size_t PACKET_MAX_LEN = (PACKET_START_LEN + PACKET_SIZE_LEN + PACKET_DATA_MAX_BIT + 
   PACKET_PARITY_LEN + PACKET_END_LEN);

// This value means that the hardware samples DECODE_SAMPLE_RATIO times within a
// bit-time. Higher value reduces the chance of hardware error but increases the
// time to decode a packet.
constexpr size_t DECODE_SAMPLE_RATIO = 4;
// The number of 0s or 1s to be considered as a valid bit. If the number of 0s
// and 1s are both less than this value, the bit is considered as invalid.
constexpr size_t DECODE_SAMPLE_RATIO_THRESHOLD = 3;


constexpr size_t QUEUE_MAX_SIZE = ((PACKET_MAX_LEN + 10) * DECODE_SAMPLE_RATIO);

constexpr size_t IR_SERVICE_TX_SIZE = 32;
constexpr size_t IR_SERVICE_RX_SIZE = 32;
constexpr size_t IR_SERVICE_BUFFER_SIZE = PACKET_MAX_LEN;
constexpr uint16_t PWM_PULSE = 16;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_PARAMS_H_
