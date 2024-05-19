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

class DelayedTask : public virtual Task {
	unsigned wakeTime;
public:
	DelayedTask(unsigned prio, task_callback_t callback, void *thisptr, void *arg, unsigned wakeTime);
	virtual ~DelayedTask();

	bool operator <(DelayedTask &task);
};

} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_DELAYEDTASK_H_ */
