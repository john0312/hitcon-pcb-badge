/*
 * Scheduler.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "Scheduler.h"

namespace hitcon {
namespace service {
namespace sched {

Scheduler scheduler;

Scheduler::Scheduler() {
}

Scheduler::~Scheduler() {
}

bool Scheduler::Queue(Task *task) {
	return tasks.Add(task);
}

bool Scheduler::Queue(DelayedTask *task) {
	return delayedTasks.Add(task);
}

bool Scheduler::Queue(PeriodicTask *task) {
	return disabledPeriodicTasks.Add(task);
}

bool Scheduler::EnablePeriodic(PeriodicTask *task) {
	if (!disabledPeriodicTasks.Remove(task))
		return false;
	return enabledPeriodicTasks.Add(task);
}

bool Scheduler::DisablePeriodic(PeriodicTask *task) {
	if (!enabledPeriodicTasks.Remove(task))
		return false;
	return disabledPeriodicTasks.Add(task);
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
