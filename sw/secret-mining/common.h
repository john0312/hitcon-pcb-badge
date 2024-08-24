#pragma once
#include <cassert>
#include <cstdint>
#ifdef __linux__
#include <endian.h>
#endif  // __linux__

constexpr bool isLittleEndian() { return __BYTE_ORDER == __LITTLE_ENDIAN; }

constexpr uint64_t rotateLeft(uint64_t x, unsigned n) {
  assert(n < 64);
  int reverse = 64 - n;
  return (x << n) | (x >> reverse);
}

constexpr void _swap(uint8_t &v1, uint8_t &v2) {
  // std::swap becomes constexpr only in C++20.
  uint8_t tmp = v1;
  v1 = v2;
  v2 = tmp;
}

constexpr uint64_t toLittleEndian(uint64_t val) {
  // use such coding to omit NVCC warning
#if __BYTE_ORDER == __LITTLE_ENDIAN
  return val;
#else
  uint8_t *ptr = reinterpret_cast<uint8_t *>(&val);

  _swap(ptr[0], ptr[7]);
  _swap(ptr[1], ptr[6]);
  _swap(ptr[2], ptr[5]);
  _swap(ptr[3], ptr[4]);
  return val;
#endif
}
