/*
 * SysTimer.cpp
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#include "SysTimer.h"
#include <main.h>

namespace hitcon {
namespace service {
namespace sched {

SysTimer::SysTimer() {
	// TODO Auto-generated constructor stub

}

SysTimer::~SysTimer() {
	// TODO Auto-generated destructor stub
}

unsigned SysTimer::GetTime() {
	return HAL_GetTick();
}

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
