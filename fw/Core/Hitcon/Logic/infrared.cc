#include "infrared.h"

/**
 * Notes:
 * - To improve performance, use xor of integers instead of inner product of
 * array.
 */

raw_packet_t encode_packet(uint8_t size_byte, uint8_t *data) {
  raw_packet_t packet;
  size_t i = 0;
  for (; i < PACKET_START_LEN; i++) {
    packet.bits[i] = PACKET_START[i];
  }
  for (size_t j = 0; j < PACKET_SIZE_LEN; j++) {
    packet.bits[i++] = (size_byte >> j) & 1;  // Little-endian
  }
  int parity = 0;
  for (int j = 0; j < size_byte; j++) {
    for (int k = 0; k < 8; k++) {
      packet.bits[i++] = (data[j] >> k) & 1;  // Little-endian
      parity ^= packet.bits[i - 1];
    }
  }
  packet.bits[i++] = parity;
  for (size_t j = 0; j < PACKET_END_LEN; j++) {
    packet.bits[i++] = PACKET_END[j];
  }
  packet.size = i;
  return packet;
}

#define QUEUE_MAX_SIZE ((PACKET_MAX_LEN + 10) * DECODE_SAMPLE_RATIO)
typedef struct {
  uint8_t buf[QUEUE_MAX_SIZE];
  size_t start, end;  // data is in [start, end), circular buffer
} queue_t;

// TODO: profile and see if using if-else is faster than modulo
#define queue_size(q) \
  (((q)->end - (q)->start + QUEUE_MAX_SIZE) % QUEUE_MAX_SIZE)

// Return -1 if the bit at pos is invalid.
// Return 1 if the bit at pos is 1.
// Return 0 if the bit at pos is 0.
int decode_bit_at_pos(queue_t *buf, size_t pos) {
  int num_ones = 0;
  for (size_t j = 0; j < DECODE_SAMPLE_RATIO; j++) {
    num_ones +=
        buf->buf[(buf->start + pos * DECODE_SAMPLE_RATIO + j) % QUEUE_MAX_SIZE];
  }
  if (num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD) return 1;
  if (DECODE_SAMPLE_RATIO - num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD) return 0;
  return -1;
}

int decode_packet(raw_packet_t *received_bits, size_t *decoded_size_byte,
                  uint8_t decoded_data[PACKET_DATA_MAX_BYTE]) {
  static queue_t buf;
  int size = 0;
  int parity = 0;

  // Append data to buffer
  for (size_t i = 0; i < received_bits->size; i++) {
    buf.buf[buf.end] = received_bits->bits[i];
    buf.end = (buf.end + 1) % QUEUE_MAX_SIZE;
  }

  // TODO: profile and optimize
  while (1) {
    // Check if buffer has enough data
    if (queue_size(&buf) < PACKET_MAX_LEN * DECODE_SAMPLE_RATIO) return 0;

    // Find start sequence
    for (size_t i = 0; i < PACKET_START_LEN; i++) {
      if (decode_bit_at_pos(&buf, i) != PACKET_START[i]) goto next;
    }

    // Find size (little-endian)
    size = 0;
    for (size_t i = 0; i < PACKET_SIZE_LEN; i++) {
      int bit = decode_bit_at_pos(&buf, PACKET_START_LEN + i);
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
            &buf, PACKET_START_LEN + PACKET_SIZE_LEN + i * 8 + j);
        if (bit == -1) goto next;
        byte |= bit << j;
        parity ^= bit;
      }
      decoded_data[i] = byte;
    }
    *decoded_size_byte = size;

    // Check parity
    if (parity !=
        decode_bit_at_pos(&buf, PACKET_START_LEN + PACKET_SIZE_LEN + size * 8))
      goto next;

    // Find end sequence
    for (size_t i = 0; i < PACKET_END_LEN; i++) {
      if (decode_bit_at_pos(&buf, PACKET_START_LEN + PACKET_SIZE_LEN +
                                      size * 8 + PACKET_PARITY_LEN + i) !=
          PACKET_END[i])
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
