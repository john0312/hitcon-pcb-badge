/*
 * PeriodicTask.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_PERIODICTASK_H_
#define HITCON_SERVICE_SCHED_PERIODICTASK_H_

#include "DelayedTask.h"

namespace hitcon {
namespace service {
namespace sched {

class PeriodicTask : public DelayedTask {
private:
	bool enabled;
	unsigned interval;
	void *savedThisptr;
	task_callback_t savedCallback;
	void AutoRequeueCb(void *arg);
public:
	PeriodicTask(unsigned prio, task_callback_t callback, void *thisptr, void *arg, unsigned interval);
	virtual ~PeriodicTask();
	void Enable();
	void Disable();
};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_PERIODICTASK_H_ */
