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
	savedCallback(savedThisptr, arg);
	if (enabled) {
		wakeTime = SysTimer::GetTime() + interval;
		scheduler.Queue((DelayedTask *)this, arg);
	}
}

PeriodicTask::~PeriodicTask() {
}

void PeriodicTask::Enable() {
	enabled = true;
}

void PeriodicTask::Disable() {
	enabled = false;
}


} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
