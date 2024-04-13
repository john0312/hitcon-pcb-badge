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

void hitcon_init_leddma(TIM_HandleTypeDef *tHandler2, DMA_HandleTypeDef *dmaCh2, TIM_HandleTypeDef *tHandler4, DMA_HandleTypeDef *dmaCh4);
void hitcon_mainloop();

#ifdef __cplusplus
}
#endif

#endif /* HITCON_HITCON_H_ */
