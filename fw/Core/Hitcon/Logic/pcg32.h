#ifndef LOGIC_PCG32_DOT_H_
#define LOGIC_PCG32_DOT_H_

#include <stdint.h>

class PCG32 {
 private:
  uint64_t state;
  static constexpr uint64_t multiplier = 6364136223846793005u;
  static constexpr uint64_t increment = 1442695040888963407u;

  uint32_t rotr32(uint32_t x, unsigned r) { return x >> r | x << (-r & 31); }

 public:
  PCG32(uint64_t seed) : state(seed + increment) { GetRandom(); }

  void MixState(uint64_t input) {
    state = state * multiplier + input;
    GetRandom();
  }

  uint32_t GetRandom() {
    uint64_t x = state;
    unsigned count = (unsigned)(x >> 59);  // 59 = 64 - 5

    state = x * multiplier + increment;
    x ^= x >> 18;                               // 18 = (64 - 27)/2
    return rotr32((uint32_t)(x >> 27), count);  // 27 = 32 - 5
  }
};

#endif  // LOGIC_PCG32_DOT_H_
