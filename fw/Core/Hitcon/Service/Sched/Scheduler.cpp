/*
 * Scheduler.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "Scheduler.h"
#include "SysTimer.h"
#include <main.h>
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
	HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
	return tasks.Add(task);
	HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

bool Scheduler::Queue(DelayedTask *task, void *arg) {
  my_assert(task);
	task->SetArg(arg);
	HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
	return delayedTasks.Add(task);
	HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
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
		HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
		DelayedTask &top = delayedTasks.Top();
		unsigned wake = top.WakeTime();
		if (wake > now)
			break;
		delayedTasks.Remove(&top);
		tasks.Add(&top);
		HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	}
}

void Scheduler::Run() {
	while (1) {
		DelayedHouseKeeping();
		HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
		if (!tasks.size())
			continue;
		Task &top = tasks.Top();
		tasks.Remove(&top);
		HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
		top.Run();
	}
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
