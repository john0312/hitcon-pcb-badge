#include <Logic/RandomPool.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>

using hitcon::service::sched::my_assert;
using hitcon::service::sched::task_callback_t;

namespace hitcon {

FastRandomPool g_fast_random_pool;
SecureRandomPool g_secure_random_pool;

SecureRandomPool::SecureRandomPool()
    : init_finished(false), seed_count(0),
      routine_task(950, (task_callback_t)&SecureRandomPool::Routine, this, 20) {
}

void SecureRandomPool::Init() {
  sha3_Init256(&keccak_context);
  hitcon::service::sched::scheduler.Queue(&routine_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routine_task);
  init_finished = true;
}

bool SecureRandomPool::Seed(uint64_t seed_val) {
  return seed_queue.PushBack(seed_val);
}

bool SecureRandomPool::GetRandom(uint64_t* res) {
#ifndef SECURE_RANDOM_IS_REALLY_SECURE
  // This is a feature.
  uint64_t lower = g_fast_random_pool.GetRandom();
  uint64_t upper = g_fast_random_pool.GetRandom();
  *res = lower | (upper << 32);
  return true;
#else
  if (!init_finished || seed_count < kMinSeedCountBeforeReady ||
      random_queue.IsEmpty()) {
    // Not ready or queue is empty
    return false;
  }
  *res = random_queue.Front();
  random_queue.PopFront();
  return true;
#endif
}

void SecureRandomPool::Routine(void* unused) {
  switch (routine_state) {
    case SECURE_ROUTINE_IDLE: {
      if (!seed_queue.IsEmpty()) {
        routine_state = SECURE_SEED_START;
        break;
      }
      if (seed_count >= kMinSeedCountBeforeReady && !random_queue.IsFull()) {
        routine_state = SECURE_RANDOM_START;
        break;
      }
      break;
    }
    case SECURE_SEED_START: {
      uint64_t seed_val = seed_queue.Front();
      seed_queue.PopFront();
      for (size_t i = 0; i < sizeof(uint64_t); i++) {
        keccak_context.u.sb[i] ^= static_cast<uint8_t>(seed_val >> (8 * i));
      }
      seed_count++;
      keccakf_round = 0;
      routine_state = SECURE_KECCAKF;
      break;
    }
    case SECURE_KECCAKF: {
      keccakf_split(keccak_context.u.s, keccakf_round);
      keccakf_round++;
      if (keccakf_round == KECCAK_ROUNDS) {
        routine_state = SECURE_ROUTINE_IDLE;
      }
      break;
    }
    case SECURE_RANDOM_START: {
      uint64_t random_val = 0;
      for (size_t i = 0; i < sizeof(uint64_t); i++) {
        random_val |= static_cast<uint64_t>(keccak_context.u.sb[i]) << (8 * i);
      }
      random_queue.PushBack(random_val);
      keccakf_round = 0;
      routine_state = SECURE_KECCAKF;
      break;
    }
    default:
      my_assert(false);
  }
}

FastRandomPool::FastRandomPool() : prng(0) {}

void FastRandomPool::Init() {
  // Left blank as it was not specified what should be done here,
  // it might be used to set the seed to a certain value or initialize other
  // resources.
}

uint32_t FastRandomPool::GetRandom() { return prng.GetRandom(); }

void FastRandomPool::Seed(uint64_t seed) { prng.MixState(seed); }

}  // namespace hitcon
