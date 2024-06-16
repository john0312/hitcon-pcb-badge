/*
 * DelayedTask.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "DelayedTask.h"

namespace hitcon {
namespace service {
namespace sched {

DelayedTask::DelayedTask(unsigned prio, task_callback_t callback, void *thisptr, void *arg, unsigned wakeTime)
		: Task(prio, callback, thisptr, arg), wakeTime(wakeTime) {
}

DelayedTask::~DelayedTask() {
}

bool DelayedTask::operator <(DelayedTask &task) {
	return wakeTime < task.wakeTime;
}

unsigned DelayedTask::WakeTime() {
	return wakeTime;
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
