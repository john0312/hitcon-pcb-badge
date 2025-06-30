/*
 * Hitcon.h
 *
 *  Created on: Jun 30, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_HITCON_H_
#define HITCON_HITCON_H_

// V1_1C  - HITCON CMT 2024 final version
// V2_0A  - HITCON CMT 2025 first prototype
// V2_1BB - HITCON CMT 2025 second prototype (blue light version)
#if !defined(V1_1C) && !defined(V2_0A) && !defined(V2_1BB)
#error "You must define at least one of: V1_1C, V2_0A, V2_1BB"
#endif

void hitcon_run();

#endif /* HITCON_HITCON_H_ */
