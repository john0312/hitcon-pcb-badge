/*
 * Hitcon.h
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_HITCON_H_
#define HITCON_HITCON_H_

#if __has_include("Config/Custom.h")
#include "Config/Custom.h"
#endif
#include "Config/Default.h"

// V1_1  - HITCON CMT 2024 attendee version
// V2_0  - HITCON CMT 2025 first prototype
// V2_1  - HITCON CMT 2025 last prototype
// V2_2  - HITCON CMT 2025 attendee version
#if !defined(V1_1) && !defined(V2_0) && !defined(V2_1) && !defined(V2_2)
#error "You must define one of: V1_1, V2_0, V2_1, V2_2"
#endif

// comment this if you want to disable dummy step generation
#define DUMMY_STEP

void hitcon_run();

#endif /* HITCON_HITCON_H_ */
