#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class SHA3_cpu {
 public:
  SHA3_cpu(size_t block);
  void init();
  void add(const uint8_t *data, size_t sz);

  std::vector<uint8_t> digest();

 private:
  // Argument buf should be at least m_buffer_size.
  void processBlock(const uint8_t *buf);

  void finish();

 private:
  uint64_t m_A[25];  // State array.
  size_t m_digestSize = 0;

  size_t m_bufferSize = 0;
  std::unique_ptr<uint8_t[]> m_blockBuffer;
  size_t m_bufferOffset = 0;

  bool m_finished = false;
};

class SHA3_cpu_batch {
 public:
  using Digest = std::vector<uint8_t>;

  SHA3_cpu_batch(size_t block);

  std::vector<Digest> calculate(
      const std::vector<std::pair<const uint8_t *, size_t>> &datas);
  size_t batchSize() const { return m_states.size(); }

 private:
  std::vector<Digest> prepareResult(size_t size);

 private:
  size_t m_digestSize = 0;
  struct State {
    uint64_t A[25];
    std::unique_ptr<uint8_t[]> blockBuffer;
  };
  std::vector<State> m_states;
};
