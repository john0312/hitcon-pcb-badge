/*
 * Scheduler.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_SCHEDULER_H_
#define HITCON_SERVICE_SCHED_SCHEDULER_H_

#include "Ds/Heap.h"
#include "Ds/Array.h"
#include "Scheduler.h"
#include "Task.h"
#include "DelayedTask.h"
#include "PeriodicTask.h"


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

class Scheduler {
	Heap<Task, 50> tasks;
	Heap<DelayedTask, 50> delayedTasks;
	Array<PeriodicTask, 50> enabledPeriodicTasks, disabledPeriodicTasks;
	void DelayedHouseKeeping();
public:
	Scheduler();
	virtual ~Scheduler();
	bool Queue(Task *task, void *arg);
	bool Queue(DelayedTask *task, void *arg);
	bool Queue(PeriodicTask *task, void *arg); // Queued tasks are disabled by default
	bool EnablePeriodic(PeriodicTask *task);
	bool DisablePeriodic(PeriodicTask *task);
	void Run();
};

extern Scheduler scheduler;

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_SCHEDULER_H_ */
