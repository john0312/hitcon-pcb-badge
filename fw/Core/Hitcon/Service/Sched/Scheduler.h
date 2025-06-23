/*
 * Scheduler.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_SCHEDULER_H_
#define HITCON_SERVICE_SCHED_SCHEDULER_H_

#include <Util/CircularQueue.h>
#include <stddef.h>

#include <cstdint>

#include "DelayedTask.h"
#include "Ds/Array.h"
#include "Ds/Heap.h"
#include "PeriodicTask.h"
#include "Scheduler.h"
#include "Task.h"

namespace hitcon {
namespace service {
namespace sched {

/*
Priority
--------

Priority ranges from 0 to 1000. 0 is not used.
Lower priority value represents tasks that will be executed in shorter
time frame (ie. higher priority).

For background computation task, we use priority 800-1000.
For real time task that has soft deadline, such as button detection, we use
priority 300-400.
For real time task that has hard deadline, such as display refresh/trigger,
we use priority 100-200.
*/

struct TaskRecord {
  Task *task;
  uint32_t startTime;
  uint32_t endTime;
};

class Scheduler {
 private:
  static constexpr size_t kAddQueueSize = 16;
  static constexpr size_t kRecordSize = 20;

  Heap<Task, 32> tasks;
  Heap<DelayedTask, 24> delayedTasks;
  Array<PeriodicTask, 24> enabledPeriodicTasks, disabledPeriodicTasks;

  // Queue used to temporarily hold calls to Queue() so we can defer heap
  // operations to later.
  /* Previous raw queue implementation.
  Task* tasksAddQueue[kAddQueueSize];
  size_t tasksAddQueueHead = 0;
  size_t tasksAddQueueTail = 0;
  DelayedTask* delayedTasksAddQueue[kAddQueueSize];
  size_t delayedTasksAddQueueHead = 0;
  size_t delayedTasksAddQueueTail = 0;*/
  CircularQueue<Task *, kAddQueueSize> tasksAddQueue;
  CircularQueue<DelayedTask *, kAddQueueSize> delayedTasksAddQueue;

  size_t totalTasks = 0;

  TaskRecord taskRecords[kRecordSize];
  size_t record_index{0};

  Task *currentTask = nullptr;

  void DelayedHouseKeeping();

 public:
  Scheduler();
  virtual ~Scheduler();
  // Can be called during interrupt.
  bool Queue(Task *task, void *arg);
  // Can be called during interrupt.
  bool Queue(DelayedTask *task, void *arg);
  // Can NOT be called during interrupt.
  bool Queue(PeriodicTask *task,
             void *arg);  // Queued tasks are disabled by default
  // Can NOT be called during interrupt.
  bool EnablePeriodic(PeriodicTask *task);
  // Can NOT be called during interrupt.
  bool DisablePeriodic(PeriodicTask *task);
  void Run();

  // Which task is running now? nullptr for nothing's running.
  Task *GetCurrentTask() { return currentTask; }

  // How many tasks has run?
  size_t GetTotalTasksRan() { return totalTasks; }
};

extern Scheduler scheduler;

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_SCHEDULER_H_ */
