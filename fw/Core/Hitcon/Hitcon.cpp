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


void hitcon_loop() {
	irSender.trigger();
	HAL_Delay(1000);
	irSender.stop();
	HAL_Delay(1000);
}
