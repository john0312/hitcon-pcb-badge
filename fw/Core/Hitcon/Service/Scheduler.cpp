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
	// TODO Auto-generated constructor stub

}

Scheduler::~Scheduler() {
	// TODO Auto-generated destructor stub
}

bool Scheduler::Queue(Task *task) {
	tasks.Add(task);
}

bool Scheduler::Queue(DelayedTask *task) {
	delayedTasks.Add(task);
}

} /* namespace service */
} /* namespace hitcon */
