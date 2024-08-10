#ifndef LOGIC_ENTROPY_HUB_DOT_H
#define LOGIC_ENTROPY_HUB_DOT_H

#include <Logic/RandomPool.h>
#include <Service/PerBoardData.h>

namespace hitcon {

// EntropyHub is in charge of seeding and making sure the RandomPools are
// ready.
class EntropyHub {
 public:
  EntropyHub();

  void Init();

  bool EntropyReady();

 private:
  // Scheduler task for running Routine(), runs every 100ms with low priority
  // (980).
  hitcon::service::sched::PeriodicTask routine_task;

  int state;
  // 0xTTTTIIII:
  // TTTT is the current task, IIII is the task information.
  // Tasks:
  // 0x0 - Init
  static constexpr int kTaskInit = 0x0;
  static constexpr int kTaskPerBoard = 0x1;
  static constexpr int kTaskInitialSeedingAdc = 0x2;
  static constexpr int kTaskInitialRounds = 0x3;
  static constexpr int kTaskFinished = 0x10;

  // How many tasks did the scheduler finished since last time?
  size_t last_sched_tasks;

  // How many times did we seed from the ADC?
  int adc_seed_count;

  // Is the random ready?
  bool random_ready;

  // Try pulling entropy from scheduler.
  bool TrySeedSched();

  // Pull entropy from secure pool and feed it into fast pool.
  bool FeedFast();

  void Routine(void* unused);

  // Accepts noise from NoiseSource.
  void AcceptNoiseFromSource(void* arg1);
};

extern EntropyHub g_entropy_hub;

}  // namespace hitcon

#endif  // LOGIC_ENTROPY_HUB_DOT_H
