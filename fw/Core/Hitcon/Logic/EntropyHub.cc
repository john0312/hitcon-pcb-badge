#include <Logic/EntropyHub.h>
#include <Service/NoiseSource.h>
#include <Service/Sched/Scheduler.h>
#include <string.h>

using hitcon::service::sched::scheduler;
using hitcon::service::sched::task_callback_t;

namespace hitcon {

EntropyHub g_entropy_hub;

namespace {

constexpr size_t kInitialRounds = 4;
constexpr size_t kPerpetualRounds = 128;
constexpr size_t kMinAdcSeedCount = 4;
constexpr size_t kMaxAdcSeedCount = 32;

}  // namespace

EntropyHub::EntropyHub()
    : routine_task(980, (task_callback_t)&EntropyHub::Routine, this, 100),
      state(0), last_sched_tasks(0), random_ready(false) {}

void EntropyHub::AcceptNoiseFromSource(void* arg1) {
  if (adc_seed_count >= kMaxAdcSeedCount) return;

  uint16_t* adc_vals = reinterpret_cast<uint16_t*>(arg1);
  // Loop from 0 to NoiseSource::kNoiseLen-1
  uint64_t seed_val = 0;
  constexpr size_t kSeedShift =
      (sizeof(uint64_t) * 8 - sizeof(uint16_t) * 8) / NoiseSource::kNoiseLen;
  for (int i = 0; i < NoiseSource::kNoiseLen; i++) {
    seed_val = seed_val << kSeedShift;
    seed_val = seed_val ^ static_cast<uint64_t>(adc_vals[i]);
  }
  bool ret = g_secure_random_pool.Seed(seed_val);
  if (ret) {
    adc_seed_count++;
  }
}

void EntropyHub::Init() {
  hitcon::service::sched::scheduler.Queue(&routine_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routine_task);

  g_noise_source.SetOnNoiseBytes(
      (task_callback_t)&EntropyHub::AcceptNoiseFromSource, this);
}

bool EntropyHub::EntropyReady() { return random_ready; }

bool EntropyHub::TrySeedSched() {
  size_t curr_tasks = scheduler.GetTotalTasksRan();
  uint32_t diff = curr_tasks - last_sched_tasks;
  g_secure_random_pool.Seed(static_cast<uint64_t>(diff));
  last_sched_tasks = curr_tasks;
  return true;
}

bool EntropyHub::FeedFast() {
  uint64_t r;
  bool ret = g_secure_random_pool.GetRandom(&r);
  if (!ret) return ret;
  g_fast_random_pool.Seed(r);
  return true;
}

void EntropyHub::Routine(void* unused) {
  int ti = state & 0x0FFFF;
  bool ret;
  // TODO: Pull entropy from Random Source.
  // TODO: Pull entropy from buttons
  // TODO: Pull entropy from IR.
  switch (state >> 16) {
    case kTaskInit:
      state = kTaskPerBoard << 16;
      break;
    case kTaskPerBoard: {
      const uint8_t* pb_rand = g_per_board_data.GetPerBoardRandom();
      uint64_t curr_rand;
      memcpy(&curr_rand, &pb_rand[ti * sizeof(uint64_t)], sizeof(uint64_t));
      ret = g_secure_random_pool.Seed(static_cast<uint64_t>(curr_rand));
      if (ret) {
        ti++;
      }
      state = (state & 0xFFFF0000) | ti;
      if (ti >= SecureRandomPool::kPerBoardRandRounds) {
        state = kTaskInitialSeedingAdc << 16;
      }
    } break;
    case kTaskInitialSeedingAdc:
      if (adc_seed_count > SecureRandomPool::kMinAdcSeedCount) {
        state = kTaskInitialRounds << 16;
      }
      break;
    case kTaskInitialRounds:
      switch (ti % 2) {
        case 0:
          ret = TrySeedSched();
          break;
        case 1:
          ret = FeedFast();
          break;
      }
      if (ret) {
        ti++;
      }
      state = (state & 0xFFFF0000) | ti;
      if (ti >= kInitialRounds) {
        state = kTaskFinished << 16;
        random_ready = true;
      }
    case kTaskFinished:
      if (ti == 1) {
        ret = TrySeedSched();
      } else if (ti == 11) {
        ret = FeedFast();
      } else {
        ret = true;
      }
      if (ret) {
        ti++;
      }
      ti = ti % kPerpetualRounds;
      state = (state & 0xFFFF0000) | ti;
      break;
  }
}

}  // namespace hitcon
