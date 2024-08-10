#ifndef HITCON_SERVICE_IR_PARAMS_H_
#define HITCON_SERVICE_IR_PARAMS_H_

#include <stddef.h>
#include <stdint.h>

namespace hitcon {
namespace ir {

// Maximum size of the payload data in a packet.
constexpr size_t MAX_PACKET_PAYLOAD_BYTES = 32;

// This value means that the hardware samples DECODE_SAMPLE_RATIO times within a
// bit-time. Higher value reduces the chance of hardware error but increases the
// time to decode a packet.
constexpr size_t DECODE_SAMPLE_RATIO = 4;
// The number of 0s or 1s to be considered as a valid bit. If the number of 0s
// and 1s are both less than this value, the bit is considered as invalid.
constexpr size_t DECODE_SAMPLE_RATIO_THRESHOLD = 3;

// Half circular size of the tx dma buffer, this is the number of uint16_t per
// interrupt (half/full).
constexpr size_t IR_SERVICE_TX_SIZE = 128;
// Half circular size of the rx dma buffer, this is the number of uint16_t per
// interrupt (half/full).
constexpr size_t IR_SERVICE_RX_SIZE = 64;
constexpr int16_t IR_PWM_TIM_CCR = 16;

constexpr size_t IR_SERVICE_RX_ON_BUFFER_SIZE = 32;

// How many rx buffer is processed per task run in IrLogic?
constexpr size_t IR_SERVICE_RX_BUFFER_PER_RUN = 4;
static_assert(IR_SERVICE_RX_ON_BUFFER_SIZE % IR_SERVICE_RX_BUFFER_PER_RUN == 0);

// Two elements represents a data bit, see PULSE_PER_HEADER_BIT.
constexpr uint8_t IR_PACKET_HEADER[] = {
    0, 0, 0, 0,
    0,              // Pad to boundary.
    1, 1, 1,        // 1.5x bit time of 1.
    0, 0, 0, 0, 0,  // 2.5x bit time of 0.
    1, 1, 1         // 1.5x bit time of 1.
};
// 1.5 2.5 1.5 times 4(decode ratio)
constexpr size_t IR_PACKET_HEADER_PACKED = 0b111'111'00000'00000'111'111;
// last bit is don't care
constexpr size_t IR_PACKET_HEADER_MASK = 0b111'110'01111'11110'011'110;

constexpr size_t PULSE_PER_DATA_BIT = 16;
constexpr size_t PULSE_PER_HEADER_BIT = PULSE_PER_DATA_BIT / 2;

// Number of elements in IR_PACKET_HEADER.
constexpr size_t IR_PACKET_HEADER_SIZE =
    sizeof(IR_PACKET_HEADER) / sizeof(IR_PACKET_HEADER[0]);
// How many elements of IR_PACKET_HEADER do we send out per DMA population run?
constexpr size_t IR_PACKET_PER_RUN = (IR_SERVICE_TX_SIZE) / 8;
constexpr size_t IR_PACKET_RUN_COUNT =
    IR_PACKET_HEADER_SIZE / IR_PACKET_PER_RUN;

static_assert((IR_SERVICE_TX_SIZE) % 8 == 0);
static_assert((IR_PACKET_HEADER_SIZE * 8) % (IR_SERVICE_TX_SIZE) == 0);

constexpr size_t IR_BITS_PER_TX_RUN = (IR_SERVICE_TX_SIZE) / PULSE_PER_DATA_BIT;
static_assert((IR_SERVICE_TX_SIZE / 2) % PULSE_PER_DATA_BIT == 0);

constexpr size_t IR_BYTE_PER_RUN = IR_SERVICE_RX_SIZE / 8;
// An integer number of run is needed to fulfill the OnBufferRecv().
static_assert(IR_SERVICE_RX_ON_BUFFER_SIZE % IR_BYTE_PER_RUN == 0);

// How many bytes in the rx buffer to be all zero for us to consider the period
// as quiet?
// For period=16, the time is 16 bit, or about 16ms.
constexpr size_t IR_LOADFACTOR_PERIOD = 16;

// How much periods do we collect for the load factor computation?
constexpr size_t IR_LOADFACTOR_SAMPLING_COUNT = 128;

// Filter with center frequency of about 10 IR_LOADFACTOR_SAMPLING_COUNT.
constexpr double LF_FC = 0.1;
constexpr double LF_ALPHA_F =
    (2.0 * 3.14159 * LF_FC) / (2.0 * 3.14159 * LF_FC + 1.0);

// Alpha for load factor low pass, in Q21.10 format.
constexpr uint32_t LF_ALPHA = (static_cast<uint32_t>(LF_ALPHA_F * (1 << 10)));
constexpr uint32_t LF_ALPHA_COMPL = (1 << 10) - LF_ALPHA;

// We're scaling the load factor because 100% is impossible, so the actual
// load factor reported is multiplied by this scale. We assumed the actual
// maximum load to result in load factor = 1.0/LF_MAX_SCALE.
constexpr uint32_t LF_MAX_SCALE = 5;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_PARAMS_H_
