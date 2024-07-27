#include <Logic/EntropyHub.h>
#include <Service/Sched/Scheduler.h>
#include <string.h>

using hitcon::service::sched::scheduler;
using hitcon::service::sched::task_callback_t;

namespace hitcon {

namespace {

constexpr size_t kPerBoardRandRounds =
    PerBoardData::kRandomLen / sizeof(uint32_t);
constexpr size_t kInitialRounds = 16;

}  // namespace

EntropyHub::EntropyHub()
    : routine_task(980, (task_callback_t)&EntropyHub::Routine, this, 100),
      state(0), last_sched_tasks(0) {}

void EntropyHub::Init() {
  hitcon::service::sched::scheduler.Queue(&routine_task, nullptr);
  hitcon::service::sched::scheduler.EnablePeriodic(&routine_task);
}

bool EntropyHub::EntropyReady() { return (state >> 16) == kTaskFinished; }

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
      uint32_t curr_rand;
      memcpy(&curr_rand, &pb_rand[ti * sizeof(uint32_t)], sizeof(uint32_t));
      ret = g_secure_random_pool.Seed(static_cast<uint64_t>(curr_rand));
      if (ret) {
        ti++;
      }
      state = (state & 0xFFFF0000) | ti;
      if (ti >= kPerBoardRandRounds) {
        state = kTaskInitialSeeding << 16;
      }
    } break;
    case kTaskInitialSeeding:
    case kTaskFinished:
      switch (ti % 4) {
        case 0:
        case 1:
        case 2:
          ret = TrySeedSched();
          break;
        case 3:
          ret = FeedFast();
          break;
      }
      if (ret) {
        ti++;
      }
      state = (state & 0xFFFF0000) | ti;
      if (ti >= kInitialRounds) {
        state = kTaskFinished;
      }
      break;
  }
}

}  // namespace hitcon
