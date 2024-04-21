/*
 * hitcon.h
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_HITCON_H_
#define HITCON_HITCON_H_

#ifdef __cplusplus
extern "C" {
#endif

void hitcon_init_leddma(TIM_HandleTypeDef *tHandler, DMA_HandleTypeDef *dmaChannel);
void hitcon_mainloop();

#ifdef __cplusplus
}
#endif

#endif /* HITCON_HITCON_H_ */
