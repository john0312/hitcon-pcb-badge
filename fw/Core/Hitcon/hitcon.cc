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
void hitcon_init_leddma(TIM_HandleTypeDef *tHandler, DMA_HandleTypeDef *dmaChannel) {
	dmatest.Init(dmaChannel, tHandler);
	dmatest.Trigger();
}

void hitcon_mainloop() {
}


