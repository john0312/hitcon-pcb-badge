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

DelayedTask::~DelayedTask() {}

bool DelayedTask::operator<(DelayedTask &task) {
  return wakeTime < task.wakeTime;
}

unsigned DelayedTask::WakeTime() { return wakeTime; }

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
