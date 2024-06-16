/*
 * DelayedTask.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_DELAYEDTASK_H_
#define HITCON_SERVICE_SCHED_DELAYEDTASK_H_

#include "Task.h"

namespace hitcon {
namespace service {
namespace sched {

class DelayedTask : public Task {
protected:
	unsigned wakeTime;
public:
	constexpr DelayedTask(unsigned prio, task_callback_t callback, void *thisptr, unsigned wakeTime)
		: Task(prio, callback, thisptr), wakeTime(wakeTime) {
	}

	virtual ~DelayedTask();

	unsigned WakeTime();

	bool operator <(DelayedTask &task);

};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_DELAYEDTASK_H_ */
