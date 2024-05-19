/*
 * Hitcon.cpp
 *
 *  Created on: May 5, 2024
 *      Author: aoaaceai
 */

#include <Hitcon.h>
#include <cstdint>
#include <IrSender.h>
#include <main.h>

hitcon::IrSender irSender;

void hitcon_init(TIM_HandleTypeDef *timer, uint32_t dmaChannel) {
	irSender.init(timer, dmaChannel);
}

static void busy_delay(uint32_t delay) {
	static uint32_t lastTick = 0;
	uint32_t currentTick;
	do {
		currentTick = HAL_GetTick();
	} while (currentTick - lastTick < delay);
	lastTick = currentTick;
}


void hitcon_loop() {
	irSender.trigger();
	busy_delay(1000);
	irSender.stop();
	busy_delay(1000);
}
