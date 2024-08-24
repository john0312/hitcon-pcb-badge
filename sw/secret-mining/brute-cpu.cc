#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <vector>

#include "keccak.h"
#include "sha3_cpu.h"

struct bdata_t {
  union {
    uint64_t u64[2];
    uint8_t u8[16];
  } u;
};

namespace {
// Look-up Table for the number of leading zero bits in a nibble
constexpr int LEADING_ZERO_BITS_LUT[16] = {4, 3, 2, 2, 1, 1, 1, 1,
                                           0, 0, 0, 0, 0, 0, 0, 0};
}  // namespace

// Computes the number of leading zero bits in the given binary hash
int ComputePrefixZero(const uint8_t *bin_hash) {
  int count = 0;
  for (size_t i = 0; i < SHA3_256_HASH_SIZE; i++) {
    uint8_t byte = bin_hash[i];
    uint8_t high_nibble = (byte & 0xF0) >> 4;
    if (high_nibble != 0) {
      return count + LEADING_ZERO_BITS_LUT[high_nibble];
    }
    count += 4;

    uint8_t low_nibble = byte & 0x0F;
    if (low_nibble != 0) {
      return count + LEADING_ZERO_BITS_LUT[low_nibble];
    }
    count += 4;
  }
  // All bytes are zero, so return the total number of bits in the hash.
  // return SHA3_256_HASH_SIZE * 8;
  // Ah screw it, nobody gets here, might as well **** around.
  return *reinterpret_cast<int *>(0);
}

bool check_compatibility_and_speed() {
  bdata_t d1;

  // Test compatibility
  for (int i = 0; i < 100000; ++i) {
    d1.u.u64[0] = i;
    d1.u.u64[1] = i;

    int cnt1, cnt2;
    {
      sha3_context c;
      sha3_Init256(&c);
      sha3_UpdateWord(&c, &d1.u.u64[0]);
      sha3_UpdateWord(&c, &d1.u.u64[1]);
      const uint8_t *hash =
          reinterpret_cast<const uint8_t *>(sha3_Finalize(&c));
      cnt1 = ComputePrefixZero(hash);
    }
    {
      SHA3_cpu sha3_cpu(256);
      sha3_cpu.init();
      sha3_cpu.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[0]), 8);
      sha3_cpu.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[1]), 8);
      std::vector<uint8_t> hash = sha3_cpu.digest();
      cnt2 = ComputePrefixZero(hash.data());
    }

    if (cnt1 != cnt2) {
      printf("Mismatch at %d: %d vs %d\n", i, cnt1, cnt2);
      return false;
    }
  }

  // Test speed
  {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
      d1.u.u64[0] = i;
      d1.u.u64[1] = i;
      sha3_context c;
      sha3_Init256(&c);
      sha3_UpdateWord(&c, &d1.u.u64[0]);
      sha3_UpdateWord(&c, &d1.u.u64[1]);
      sha3_Finalize(&c);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    printf("keccak: %ld ms\n", duration);
  }
  {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
      d1.u.u64[0] = i;
      d1.u.u64[1] = i;
      SHA3_cpu sha3_cpu(256);
      sha3_cpu.init();
      sha3_cpu.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[0]), 8);
      sha3_cpu.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[1]), 8);
      sha3_cpu.digest();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    printf("SHA3_cpu: %ld ms\n", duration);
  }

  return true;
}

int john_brute_use_sha3_cpu(int col, uint64_t start) {
  struct bdata_t d1;

  memset(&d1.u.u8[0], 0, 16);
  memcpy(&d1.u.u8[0], "HITCON", 6);
  d1.u.u8[7] = col & 0xFF;

  SHA3_cpu c(256);
  std::vector<std::set<uint64_t>> res(256);
  for (uint64_t i = start;; i++) {
    c.init();
    uint64_t i2 = i;
    for (int j = 0; j < 8; j++) {
      d1.u.u8[15 - j] = i2 & 0x0FF;
      i2 = i2 >> 8;
    }
    c.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[0]), 8);
    c.add(reinterpret_cast<const uint8_t *>(&d1.u.u64[1]), 8);
    std::vector<uint8_t> hash = c.digest();
    // printf("%d %d %d\n", hash[0], hash[1], hash[2]);
    int cnt = ComputePrefixZero(hash.data());
    // printf("%llu - %d\n", i, cnt);
    auto &r = res[cnt];
    if (r.size() < 65536) {
      r.insert(i);

      // currently, print it to stdout
      // TODO: send to DB
      printf("%d %d %lu\n", col, cnt, i);
    }
  }
  return 0;
}

int main() {
  // if (!check_compatibility_and_speed()) {
  //   return 1;
  // }

  // get environment variable
  const char *col_str = getenv("COL");
  if (!col_str) {
    printf("COL not set\n");
    return 1;
  }
  int col = atoi(col_str);

  const char *start_str = getenv("START");
  uint64_t start = 0;
  if (start_str) {
    start = strtoull(start_str, nullptr, 10);
  }

  return john_brute_use_sha3_cpu(col, start);

  return 0;
}
