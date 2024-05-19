/*
 * Scheduler.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "Scheduler.h"

namespace hitcon {
namespace service {

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

} /* namespace service */
} /* namespace hitcon */
