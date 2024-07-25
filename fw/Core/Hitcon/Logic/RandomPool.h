#ifndef LOGIC_RANDOM_POOL_DOT_H_
#define LOGIC_RANDOM_POOL_DOT_H_

#include <Logic/keccak.h>
#include <Logic/pcg32.h>
#include <Service/Sched/Scheduler.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

class SecureRandomPool;
class FastRandomPool;

// SecureRandomPool maintains a pool of entropy in a keccak state, and everytime
// randomness is requested it'll run the keccakf() and output the bytes. This
// should be used for anything that needs cryptography randomness and doesn't
// mind being slow.
class SecureRandomPool {
 public:
  SecureRandomPool();

  void Init();

  // Add entropy to the secure random pool.
  // Returns true if successful, otherwise the pool is busy (or not ready yet)
  // and the caller should retry later on. Generally this adds the seed to
  // seeding queue and Routine() will take care of the rest.
  bool Seed(uint64_t seed_val);

  // Pull entropy from the secure random pool.
  // Return true if successful and the randomness will be in res. Otherwise the
  // pool is busy (or not ready yet) and the caller should retry later on.
  // Generally this involves pulling from the random queue and Routine() is in
  // charge of filling the queue.
  bool GetRandom(uint64_t* res);

  // Will be called routinely by the scheduler, and will try to empty the seed
  // queue first then fill the random queue. Each invocation is limited to 1
  // keccakf() run due to scheduling.
  void Routine(void* unused);

 private:
  // Try to seed once into the state if there's seed in the queue.
  // Seeding involves xor'ing the corresponding bytes in the state then run
  // keccakf(). Return true if one such operation has been done and that signals
  // to the caller that we can't run another keccakf() in the same Routine()
  // run.
  bool TrySeed();

  // Try to pull one random out if we've enough entropy considering seed_count
  // and there's room in the random queue. This involves getting the
  // corresponding bytes in the state. After getting the bytes, keccakf() is
  // invoked. Return true if one such operation has been done and that signals
  // to the caller that we can't run another keccakf() in the same Routine()
  // run.
  bool TryRandom();

  // Set to true after Init.
  bool init_finished;

  // The internal keccak state.
  sha3_context keccak_context;

  // Number of times we've been seeded.
  int seed_count;

  // Scheduler task for running Routine(), runs every 20ms with low priority
  // (950).
  hitcon::service::sched::PeriodicTask routine_task;

  // A circular queue for holding the to be seeded values.
  static constexpr size_t kSeedQueueSize = 8;
  uint64_t seed_queue[kSeedQueueSize];
  size_t seed_queue_head;
  size_t seed_queue_tail;

  // A circular queue for holding the newly minted random values.
  static constexpr size_t kRandomQueueSize = 8;
  uint64_t random_queue[kRandomQueueSize];
  size_t random_queue_head;
  size_t random_queue_tail;

  // Must be seeded this amount of times before GetRandom() will be ready.
  static constexpr int kMinSeedCountBeforeReady = 64;
};

// Fast random pool uses the PCG32 for faster but non-secure random generation.
class FastRandomPool {
 public:
  FastRandomPool();

  void Init();

  // Never blocks, simply retrieves the random.
  uint32_t GetRandom();

  // Add to the PRNG state, aka MixState().
  void Seed(uint64_t seed);

 private:
  PCG32 prng;
};

extern FastRandomPool g_fast_random_pool;
extern SecureRandomPool g_secure_random_pool;

}  // namespace hitcon

#endif  // LOGIC_RANDOM_POOL_DOT_H_
