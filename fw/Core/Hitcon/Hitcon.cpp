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
	irSender.trigger();
}

