#include "sha3_cpu.h"

#include <omp.h>

#include <array>
#include <cstdlib>

#include "common.h"

namespace {
constexpr size_t idx(size_t x) { return x % 5; }

constexpr size_t idx(size_t x, size_t y) { return idx(x) + 5 * idx(y); }

// Array of indicies and rotation for P and Pi phases.
constexpr std::array<std::pair<uint8_t, uint8_t>, 24> g_ppi_aux = {
    {{6, 44},  {12, 43}, {18, 21}, {24, 14}, {3, 28},  {9, 20},
     {10, 3},  {16, 45}, {22, 61}, {1, 1},   {7, 6},   {13, 25},
     {19, 8},  {20, 18}, {4, 27},  {5, 36},  {11, 10}, {17, 15},
     {23, 56}, {2, 62},  {8, 55},  {14, 39}, {15, 41}, {21, 2}}};

// Values, required for the last iota phase.
constexpr std::array<uint64_t, 24> g_iota_aux = {
    0x0000000000000001L, 0x0000000000008082L, 0x800000000000808aL,
    0x8000000080008000L, 0x000000000000808bL, 0x0000000080000001L,
    0x8000000080008081L, 0x8000000000008009L, 0x000000000000008aL,
    0x0000000000000088L, 0x0000000080008009L, 0x000000008000000aL,
    0x000000008000808bL, 0x800000000000008bL, 0x8000000000008089L,
    0x8000000000008003L, 0x8000000000008002L, 0x8000000000000080L,
    0x000000000000800aL, 0x800000008000000aL, 0x8000000080008081L,
    0x8000000000008080L, 0x0000000080000001L, 0x8000000080008008L};

void updateState(uint64_t A[25]) {
  for (int round = 0; round < 24; ++round) {
    // Thetta phase
    uint64_t C[25];
    for (size_t x = 0; x < 5; x++) {
      C[x] = A[idx(x, 0)] ^ A[idx(x, 1)] ^ A[idx(x, 2)] ^ A[idx(x, 3)] ^
             A[idx(x, 4)];
    }

    for (size_t x = 0; x < 5; ++x) {
      uint64_t D = C[idx(x + 5 - 1)] ^ rotateLeft(C[idx(x + 1)], 1);
      for (int y = 0; y < 5; ++y) {
        A[idx(x, y)] ^= D;
      }
    }

    // P and Pi phases
    // First element remains the same.
    C[0] = A[0];
    for (size_t i = 0; i < 24; ++i) {
      C[i + 1] = rotateLeft(A[g_ppi_aux[i].first], g_ppi_aux[i].second);
    }

    // Ksi phase
    for (size_t x = 0; x < 5; ++x) {
      for (size_t y = 0; y < 5; ++y) {
        A[idx(x, y)] = C[idx(x, y)] ^ (~C[idx(x + 1, y)] & C[idx(x + 2, y)]);
      }
    }

    // Iota phase
    A[0] ^= g_iota_aux[round];
  }
}

void processSingleBlock(uint64_t A[25], const uint8_t *data, size_t size) {
  assert(size % 8 == 0);
  for (unsigned int i = 0, ei = size / 8; i < ei; ++i) {
    A[i] ^= toLittleEndian(reinterpret_cast<const uint64_t *>(data)[i]);
  }

  updateState(A);
}

void addPadding(uint8_t *begin, uint8_t *end) {
  if (std::next(begin) == end) {
    *begin = 0x86;
    return;
  }

  *begin++ = 0x06;
  *--end = 0x80;
  std::fill(begin, end, 0);
}

void copyLittleEndian64(const uint64_t A[25], uint8_t *data, size_t size) {
  if (isLittleEndian()) {
    // Help the compiler to recognize a simple memcpy.
    const uint8_t *A8 = reinterpret_cast<const uint8_t *>(A);
    std::copy(A8, A8 + size, data);
    return;
  }

  uint64_t *data64 = reinterpret_cast<uint64_t *>(data);
  for (; size >= 8; size -= 8, ++A, ++data64) {
    *data64 = toLittleEndian(*A);
  }
  const uint8_t *A8 = reinterpret_cast<const uint8_t *>(A);
  uint8_t *data8 = reinterpret_cast<uint8_t *>(data64);
  std::copy(A8, A8 + size, data8);
}

}  // namespace

SHA3_cpu::SHA3_cpu(size_t block)
    : m_digestSize(block / 8), m_bufferSize(200 - 2 * m_digestSize),
      m_blockBuffer(static_cast<uint8_t *>(
          aligned_alloc(sizeof(uint64_t), m_bufferSize))) {
  assert(m_digestSize * 8 == block);
  init();
}

void SHA3_cpu::init() {
  std::fill(std::begin(m_A), std::end(m_A), uint64_t(0));
  m_bufferOffset = 0;

  m_finished = false;
}

void SHA3_cpu::add(const uint8_t *data, size_t sz) {
  assert(!m_finished && "Init should be called");
  while (sz != 0) {
    if (sz < m_bufferSize - m_bufferOffset) {
      std::copy(data, data + sz, m_blockBuffer.get() + m_bufferOffset);
      m_bufferOffset += sz;
      return;
    }

    if (m_bufferOffset == 0) {
      processBlock(data);
      sz -= m_bufferSize;
      data += m_bufferSize;
      continue;
    }

    size_t dataSize = m_bufferSize - m_bufferOffset;
    std::copy(data, data + dataSize, m_blockBuffer.get() + m_bufferOffset);
    processBlock(m_blockBuffer.get());
    m_bufferOffset = 0;
    sz -= dataSize;
    data += dataSize;
  }
}

void SHA3_cpu::finish() {
  addPadding(m_blockBuffer.get() + m_bufferOffset,
             m_blockBuffer.get() + m_bufferSize);
  processBlock(m_blockBuffer.get());
  m_bufferOffset = 0;
}

std::vector<uint8_t> SHA3_cpu::digest() {
  if (!m_finished) {
    finish();
    m_finished = true;
  }
  std::vector<uint8_t> result(m_digestSize);
  copyLittleEndian64(m_A, result.data(), result.size());
  return result;
}

void SHA3_cpu::processBlock(const uint8_t *buf) {
  processSingleBlock(m_A, buf, m_bufferSize);
}

SHA3_cpu_batch::SHA3_cpu_batch(size_t block) : m_digestSize(block / 8) {
  assert(m_digestSize * 8 == block);
  unsigned threads = omp_get_num_procs();
  threads = threads == 0 ? 2 : threads;
  m_states.resize(threads);
  for (auto &val : m_states) {
    val.blockBuffer.reset(new uint8_t[200 - 2 * m_digestSize]);
  }
}

std::vector<SHA3_cpu_batch::Digest> SHA3_cpu_batch::calculate(
    const std::vector<std::pair<const uint8_t *, size_t>> &datas) {
  auto result = prepareResult(datas.size());
#pragma omp parallel num_threads(m_states.size())
  {
    int tid = omp_get_thread_num();
    auto &state = m_states[tid];
    size_t blockSize = 200 - 2 * m_digestSize;
    int nthreads = omp_get_num_threads();
    for (size_t i = tid; i < datas.size(); i += nthreads) {
      std::fill(std::begin(state.A), std::end(state.A), uint64_t(0));
      size_t sizeLeft = datas[i].second;
      const uint8_t *data = datas[i].first;

      while (true) {
        if (sizeLeft < blockSize) {
          std::copy(data, data + sizeLeft, state.blockBuffer.get());
          addPadding(state.blockBuffer.get() + sizeLeft,
                     state.blockBuffer.get() + blockSize);
          processSingleBlock(state.A, state.blockBuffer.get(), blockSize);
          copyLittleEndian64(state.A, result[i].data(), m_digestSize);
          break;
        }
        processSingleBlock(state.A, data, blockSize);
        data += blockSize;
        sizeLeft -= blockSize;
      }
    }
  }
  return result;
}

std::vector<SHA3_cpu_batch::Digest> SHA3_cpu_batch::prepareResult(size_t size) {
  std::vector<Digest> result;
  result.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    result.emplace_back(m_digestSize);
  }
  return result;
}
