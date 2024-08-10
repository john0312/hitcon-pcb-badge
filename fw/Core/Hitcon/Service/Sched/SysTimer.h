/*
 * SysTimer.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_SYSTIMER_H_
#define HITCON_SERVICE_SCHED_SYSTIMER_H_

namespace hitcon {
namespace service {
namespace sched {

class SysTimer {
 public:
  SysTimer();
  virtual ~SysTimer();
  static unsigned GetTime();
};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_SYSTIMER_H_ */
