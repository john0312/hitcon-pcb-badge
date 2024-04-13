/*
 * hitcon.cc
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#include "DmaTest.h"
#include "hitcon.h"
#include "main.h"

static hitcon::DmaTest dmatest;
void hitcon_init_leddma(TIM_HandleTypeDef *tHandler2, DMA_HandleTypeDef *dmaCh2, TIM_HandleTypeDef *tHandler4, DMA_HandleTypeDef *dmaCh4) {
	DMA_HandleTypeDef *channels[2] = {dmaCh4, dmaCh2};
	TIM_HandleTypeDef *timerHandlers[2] = {tHandler4, tHandler2};
	dmatest.Init(channels, timerHandlers);
	dmatest.Trigger();
}

void hitcon_mainloop() {
}


