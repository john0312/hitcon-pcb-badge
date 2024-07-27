/*
 * Task.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_TASK_H_
#define HITCON_SERVICE_SCHED_TASK_H_

#include <Service/Sched/Checks.h>

namespace hitcon {
namespace service {
namespace sched {

typedef void (*task_callback_t)(void *thisptr, void *arg);

class Task {
protected:
	unsigned prio;
	task_callback_t callback;
	void *thisptr, *arg;
	bool in_queue = false;
public:
	// For prio, see Scheduler.h
	constexpr Task(unsigned prio, task_callback_t callback, void *thisptr) : prio(prio), callback(callback), thisptr(thisptr), arg(nullptr), in_queue(false) {
	}

	// No copy
	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	// No move constructor
	Task(Task&&) = delete;
	Task& operator=(Task&&) = delete;

	virtual ~Task();
	bool operator ==(Task &task);
	virtual bool operator <(Task &task);
	void Run();
	void SetArg(void *arg);

	// Must be called whenever entering task or delayedTask queue.
	// This is for debugging double Add() or Remove().
	inline void EnterQueue() {
		if (in_queue) AssertOverflow();
		in_queue = true;
	}

	// Must be called whenever leaving task or delayedTask queue.
	// This is for debugging double Add() or Remove().
	inline void ExitQueue() {
		if (!in_queue) AssertOverflow();
		in_queue = false;
	}
};

} /* namespace sched */
} /* namespcae service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_TASK_H_ */
