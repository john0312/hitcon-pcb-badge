#include <stddef.h>
#include <stdint.h>

/**
 * First version.
 *
 * The time unit is called "bit-time". A bit 0 or 1 is sent in 1 bit-time.
 *
 * The structure of a packet is:
 * start: 11111110000000111 (7*'1', 7*'0', 3*'1')
 * size: (5 bits, little-endian) (should not be 0)
 * data: (size * 8 bits) (little-endian)
 * parity: (1 bit, even)
 * end: 1111111 (7*'1')
 */

#define PACKET_START \
  (uint8_t[]) { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 }
#define PACKET_START_LEN (sizeof(PACKET_START) / sizeof(uint8_t))
#define PACKET_SIZE_LEN 5
#define PACKET_DATA_MAX_BYTE 32
#define PACKET_DATA_MAX_BIT (PACKET_DATA_MAX_BYTE * 8)
#define PACKET_PARITY_LEN 1
#define PACKET_END \
  (uint8_t[]) { 1, 1, 1, 1, 1, 1, 1 }
#define PACKET_END_LEN (sizeof(PACKET_END) / sizeof(uint8_t))
#define PACKET_MAX_LEN                                        \
  (PACKET_START_LEN + PACKET_SIZE_LEN + PACKET_DATA_MAX_BIT + \
   PACKET_PARITY_LEN + PACKET_END_LEN)

// This value means that the hardware samples DECODE_SAMPLE_RATIO times within a
// bit-time. Higher value reduces the chance of hardware error but increases the
// time to decode a packet.
#define DECODE_SAMPLE_RATIO 4
// The number of 0s or 1s to be considered as a valid bit. If the number of 0s
// and 1s are both less than this value, the bit is considered as invalid.
#define DECODE_SAMPLE_RATIO_THRESHOLD 3

typedef struct {
  size_t size;
  uint8_t bits[PACKET_MAX_LEN * DECODE_SAMPLE_RATIO];
} raw_packet_t;

// Encode data into a packet. data is the "byte" array to be sent. The returned
// packet is "bit" array.
raw_packet_t encode_packet(uint8_t size_byte, uint8_t *data);

// A stateful function that takes some bits decode the data if one packet
// is complete. Note that the sample rate of data is not 1 bit-time but
// DECODE_SAMPLE_RATIO times within a bit-time.
// It's important that data contains "bits" not "bytes".
// If not packet is decoded, return 0. Otherwise, return 1.
// Note that this function should always be called even if there seems to be
// no new data. This is because the function only decodes the data when buffer
// reaches a certain size. If the buffer is not filled, the function will not
// decode the data.
int decode_packet(raw_packet_t *received_bits, size_t *decoded_size_byte,
                  uint8_t decoded_data[PACKET_DATA_MAX_BYTE]);
