/*
 * Scheduler.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHEDULER_H_
#define HITCON_SERVICE_SCHEDULER_H_

#include "Ds/Heap.h"
#include "Scheduler.h"
#include "Task.h"
#include "DelayedTask.h"


namespace hitcon {
namespace service {

class Scheduler {
	Heap<Task, 50> tasks;
	Heap<DelayedTask, 50> delayedTasks;
public:
	Scheduler();
	virtual ~Scheduler();
	bool Queue(Task *task);
	bool Queue(DelayedTask *task);
};

} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHEDULER_H_ */
