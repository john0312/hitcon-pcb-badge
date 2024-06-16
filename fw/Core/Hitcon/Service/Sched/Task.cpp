/*
 * Task.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "Task.h"

namespace hitcon {
namespace service {
namespace sched {

Task::~Task() {

}

bool Task::operator ==(Task &task) {
	return &task == this;
}

bool Task::operator <(Task &task) {
	return prio < task.prio;
}

void Task::Run() {
	callback(thisptr, arg);
}

void Task::SetArg(void *arg) {
	this->arg = arg;
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
