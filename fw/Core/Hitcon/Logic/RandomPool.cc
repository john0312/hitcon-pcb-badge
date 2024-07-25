#include <Logic/RandomPool.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>

using hitcon::service::sched::task_callback_t;

namespace hitcon {

FastRandomPool g_fast_random_pool;
SecureRandomPool g_secure_random_pool;

SecureRandomPool::SecureRandomPool()
    : init_finished(false), seed_count(0),
      routine_task(950, (task_callback_t)&SecureRandomPool::Routine, this, 20),
      seed_queue_head(0), seed_queue_tail(0), random_queue_head(0),
      random_queue_tail(0) {}

void SecureRandomPool::Init() {
  sha3_Init256(&keccak_context);
  hitcon::service::sched::scheduler.Queue(&routine_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routine_task);
  init_finished = true;
}

bool SecureRandomPool::Seed(uint64_t seed_val) {
  if (seed_queue_head == (seed_queue_tail + 1) % kSeedQueueSize) {
    // Queue is full
    return false;
  }
  seed_queue[seed_queue_tail] = seed_val;
  seed_queue_tail = (seed_queue_tail + 1) % kSeedQueueSize;
  return true;
}

bool SecureRandomPool::GetRandom(uint64_t* res) {
  if (!init_finished || seed_count < kMinSeedCountBeforeReady ||
      random_queue_head == random_queue_tail) {
    // Not ready or queue is empty
    return false;
  }
  *res = random_queue[random_queue_head];
  random_queue_head = (random_queue_head + 1) % kRandomQueueSize;
  return true;
}

void SecureRandomPool::Routine(void* unused) {
  if (TrySeed()) {
    return;
  }
  if (TryRandom()) {
    return;
  }
}

bool SecureRandomPool::TrySeed() {
  if (seed_queue_head == seed_queue_tail) {
    // Queue is empty
    return false;
  }
  uint64_t seed_val = seed_queue[seed_queue_head];
  seed_queue_head = (seed_queue_head + 1) % kSeedQueueSize;
  for (size_t i = 0; i < sizeof(uint64_t); i++) {
    keccak_context.u.sb[i] ^= static_cast<uint8_t>(seed_val >> (8 * i));
  }
  keccakf(keccak_context.u.s);
  seed_count++;
  return true;
}

bool SecureRandomPool::TryRandom() {
  if (seed_count < kMinSeedCountBeforeReady ||
      random_queue_head == (random_queue_tail + 1) % kRandomQueueSize) {
    // Not ready or queue is full
    return false;
  }
  uint64_t random_val = 0;
  for (size_t i = 0; i < sizeof(uint64_t); i++) {
    random_val |= static_cast<uint64_t>(keccak_context.u.sb[i]) << (8 * i);
  }
  keccakf(keccak_context.u.s);
  random_queue[random_queue_tail] = random_val;
  random_queue_tail = (random_queue_tail + 1) % kRandomQueueSize;
  return true;
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
