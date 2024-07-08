#ifdef HITCON_TEST_MODE

#include <stdio.h>
#include <stdlib.h>

#include "infrared.h"

void test_encode() {
  uint8_t data[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
  raw_packet_t packet = encode_packet(16, data);

  size_t i = 0;

  printf("[encode]\n");
  printf("start: ");
  for (; i < PACKET_START_LEN; i++) {
    printf("%d%c", packet.bits[i], i == PACKET_START_LEN - 1 ? '\n' : ' ');
  }

  printf("size: ");
  for (size_t j = 0; j < PACKET_SIZE_LEN; j++) {
    printf("%d%c", packet.bits[i++], j == PACKET_SIZE_LEN - 1 ? '\n' : ' ');
  }

  printf("data:\n");
  for (int j = 0; j < 16; j++) {
    for (int k = 0; k < 8; k++) {
      printf("%d ", packet.bits[i++]);
    }
    printf(" (should be 0x%02x)\n", data[j]);
  }

  printf("parity: %d\n", packet.bits[i++]);

  printf("end: ");
  for (size_t j = 0; j < PACKET_END_LEN; j++) {
    printf("%d%c", packet.bits[i++], j == PACKET_END_LEN - 1 ? '\n' : ' ');
  }
}

void test_decode() {
  uint8_t _data[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                       0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
  raw_packet_t _packet = encode_packet(16, _data);

  // prepare received packet (upscaled by 4 times and simulate noise)
  raw_packet_t packet;
  for (size_t i = 0; i < _packet.size; i++) {
    packet.bits[i * 4 + 0] = _packet.bits[i];
    packet.bits[i * 4 + 1] = _packet.bits[i];
    packet.bits[i * 4 + 2] = _packet.bits[i];
    packet.bits[i * 4 + 3] = _packet.bits[i];
    // randomly flip bits
    int r = rand() % 5;
    if (r != 4) {
      packet.bits[i * 4 + r] = !packet.bits[i * 4 + r];
    }
  }
  packet.size = _packet.size * 4;

  size_t decoded_byte_size;
  uint8_t decoded_data[32];
  size_t batch_size = 5;  // change me
  size_t i = 0;
  for (; i < 2 * PACKET_MAX_LEN * DECODE_SAMPLE_RATIO; i += batch_size) {
    raw_packet_t batch;
    batch.size = 0;
    for (size_t j = 0; j < batch_size; j++) {
      batch.bits[j] = (i + j < packet.size) ? packet.bits[i + j] : rand() % 2;
      batch.size++;
    }

    if (decode_packet(&batch, &decoded_byte_size, decoded_data)) {
      printf("[decode]\n");
      printf("decoded size: %zu\n", decoded_byte_size);
      printf("decoded data:\n");
      for (size_t j = 0; j < decoded_byte_size; j++) {
        printf("0x%02x ", decoded_data[j]);
      }
      printf("\n");
    }
  }
}

int main() {
  test_encode();
  puts("");
  test_decode();

  return 0;
}

#endif
