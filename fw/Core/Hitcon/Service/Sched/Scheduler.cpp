/*
 * Scheduler.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "Scheduler.h"
#include "SysTimer.h"

namespace hitcon {
namespace service {
namespace sched {

Scheduler scheduler;

Scheduler::Scheduler() {
}

Scheduler::~Scheduler() {
}

void my_assert(bool expr) {
  if (!expr) {
      ((char*)nullptr)[0] = 0;
  }
}

bool Scheduler::Queue(Task *task, void *arg) {
	my_assert(task);
	task->SetArg(arg);
	return tasks.Add(task);
}

bool Scheduler::Queue(DelayedTask *task, void *arg) {
  my_assert(task);
	task->SetArg(arg);
	return delayedTasks.Add(task);
}

bool Scheduler::Queue(PeriodicTask *task, void *arg) {
  my_assert(task);
	task->SetArg(arg);
	return disabledPeriodicTasks.Add(task);
}

bool Scheduler::EnablePeriodic(PeriodicTask *task) {
	if (!disabledPeriodicTasks.Remove(task))
		return false;
	if (!enabledPeriodicTasks.Add(task))
		return false;
	delayedTasks.Add(task);
	task->Enable();
	return true;
}

bool Scheduler::DisablePeriodic(PeriodicTask *task) {
	if (!enabledPeriodicTasks.Remove(task))
		return false;
	if (!disabledPeriodicTasks.Add(task))
		return false;
	task->Disable();
	return true;
}

void Scheduler::DelayedHouseKeeping() {
	unsigned now = SysTimer::GetTime();
	while (delayedTasks.size()) {
		DelayedTask &top = delayedTasks.Top();
		unsigned wake = top.WakeTime();
		if (wake > now)
			break;
		delayedTasks.Remove(&top);
		tasks.Add(&top);
	}
}

void Scheduler::Run() {
	while (1) {
		DelayedHouseKeeping();
		if (!tasks.size())
			continue;
		Task &top = tasks.Top();
		tasks.Remove(&top);
		top.Run();
	}
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
