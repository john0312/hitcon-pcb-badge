#include <Logic/IrLogic.h>

namespace hitcon {

IrLogic::IrLogic() {}

void IrLogic::Init() {
  // TODO
}

void IrLogic::OnBufferReceived(uint8_t* buffer) {
  // TODO
}

void IrLogic::SetOnPacketReceived(callback_t callback, void* callback_arg1) {
  // TODO
}

void IrLogic::EncodePacket(uint8_t *data, size_t len, IrPacket &packet) {
  size_t i = 0;
  for (; i < PACKET_START_LEN; i++) {
	packet.data_[i] = PACKET_START[i];
  }
  for (size_t j = 0; j < PACKET_SIZE_LEN; j++) {
	packet.data_[i++] = (len >> j) & 1; // Little-endian
  }
  int parity = 0;
  for (size_t j = 0; j < len; j++) {
	for (size_t k = 0; k < 8; k++) {
	  packet.data_[i++] = (data[j] >> k) & 1; // Little-endian
	  parity ^= packet.data_[i - 1];
	}
  }
  packet.data_[i++] = parity;
  for (size_t j = 0; j < PACKET_END_LEN; j++) {
	packet.data_[i++] = PACKET_END[j];
  }
  packet.size_ = i;
}

#define QUEUE_MAX_SIZE ((PACKET_MAX_LEN + 10) * DECODE_SAMPLE_RATIO)
typedef struct {
  uint8_t buf[QUEUE_MAX_SIZE];
  size_t start, end; // data is in [start, end), circular buffer
  size_t size() {
	  return (QUEUE_MAX_SIZE + end - start) % QUEUE_MAX_SIZE;
  }
} queue_t;

// Return -1 if the bit at pos is invalid.
// Return 1 if the bit at pos is 1.
// Return 0 if the bit at pos is 0.
int decode_bit_at_pos(queue_t &buf, size_t pos) {
  int num_ones = 0;
  for (size_t j = 0; j < DECODE_SAMPLE_RATIO; j++) {
    num_ones +=
        buf.buf[(buf.start + pos * DECODE_SAMPLE_RATIO + j) % QUEUE_MAX_SIZE];
  }
  if (num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD)
    return 1;
  if (DECODE_SAMPLE_RATIO - num_ones >= DECODE_SAMPLE_RATIO_THRESHOLD)
    return 0;
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
	if (buf.size() < PACKET_MAX_LEN * DECODE_SAMPLE_RATIO)
	  return 0;

	// Find start sequence
	for (size_t i = 0; i < PACKET_START_LEN; i++) {
	  if (decode_bit_at_pos(buf, i) != PACKET_START[i])
		goto next;
	}

	// Find size (little-endian)
	size = 0;
	for (size_t i = 0; i < PACKET_SIZE_LEN; i++) {
	  int bit = decode_bit_at_pos(buf, PACKET_START_LEN + i);
	  if (bit == -1)
		goto next;
	  size |= bit << i;
	}
	if (size == 0)
	  goto next;

	// Parse data (little-endian)
	parity = 0;
	for (int i = 0; i < size; i++) {
	  uint8_t byte = 0;
	  for (int j = 0; j < 8; j++) {
		int bit = decode_bit_at_pos(buf, PACKET_START_LEN + PACKET_SIZE_LEN +
											  i * 8 + j);
		if (bit == -1)
		  goto next;
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
	  if (decode_bit_at_pos(buf, PACKET_START_LEN + PACKET_SIZE_LEN +
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

}  // namespace hitcon
