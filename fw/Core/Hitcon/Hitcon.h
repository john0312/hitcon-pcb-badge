/*
 * Hitcon.h
 *
 *  Created on: May 5, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_HITCON_H_
#define HITCON_HITCON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <main.h>

void hitcon_init(TIM_HandleTypeDef *timer, uint32_t dmaChannel);

void hitcon_loop();

#ifdef __cplusplus
}
#endif

#endif /* HITCON_HITCON_H_ */
