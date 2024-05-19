/*
 * PeriodicTask.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "PeriodicTask.h"
#include "Scheduler.h"
#include "SysTimer.h"

namespace hitcon {
namespace service {
namespace sched {

void PeriodicTask::AutoRequeueCb(void *arg) {
	callback(savedThisptr, arg);
	wakeTime = SysTimer::GetTime();
	scheduler.Queue((DelayedTask *)this);
}

PeriodicTask::PeriodicTask(unsigned prio, task_callback_t callback, void *thisptr, void *arg, unsigned interval)
		: DelayedTask(prio, (task_callback_t)&PeriodicTask::AutoRequeueCb, (void *)this, arg, 0),
		  interval(interval),
		  savedThisptr(thisptr) {
}

PeriodicTask::~PeriodicTask() {
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
