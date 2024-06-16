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
