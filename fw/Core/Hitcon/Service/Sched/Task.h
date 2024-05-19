/*
 * Task.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_TASK_H_
#define HITCON_SERVICE_SCHED_TASK_H_

namespace hitcon {
namespace service {
namespace sched {

typedef void (*task_callback_t)(void *thisptr, void *arg);

class Task {
private:
	unsigned tid;
protected:
	unsigned prio;
	task_callback_t callback;
	void *thisptr, *arg;
public:
	Task(unsigned prio, task_callback_t callback, void *thisptr, void *arg);
	virtual ~Task();
	unsigned GetTid();
	bool operator ==(Task &task);
	bool operator ==(unsigned &tid);
	virtual bool operator <(Task &task);
};

} /* namespace sched */
} /* namespcae service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_TASK_H_ */
