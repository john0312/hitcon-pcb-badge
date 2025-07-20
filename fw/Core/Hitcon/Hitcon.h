/*
 * Hitcon.h
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_HITCON_H_
#define HITCON_HITCON_H_

// V1_1  - HITCON CMT 2024 attendee version
// V2_0  - HITCON CMT 2025 first prototype
// V2_1  - HITCON CMT 2025 attendee version
#if !defined(V1_1) && !defined(V2_0) && !defined(V2_1)
#error "You must define at least one of: V1_1, V2_0, V2_1"
#endif

void hitcon_run();

#endif /* HITCON_HITCON_H_ */
