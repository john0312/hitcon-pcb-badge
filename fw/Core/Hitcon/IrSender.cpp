/*
 * IrSender.cpp
 *
 *  Created on: May 5, 2024
 *      Author: aoaaceai
 */

#include "IrSender.h"
#include "main.h"
#include "IrLib.h"

namespace hitcon {

const uint32_t pwmSz = 3;

uint32_t pwmData[pwmSz];

IrSender::IrSender() {

}

void IrSender::init(TIM_HandleTypeDef *timer, uint32_t dmaChannel) {
	this->timer = timer;
	this->dmaChannel = dmaChannel;
	for (unsigned i = 0; i < pwmSz; ++i)
		pwmData[i] = 100;
	HAL_TIM_Base_Start(timer);
}

void IrSender::send(uint32_t size, uint8_t *data) {
	encode_packet(size, data, &sendingPacket);
}

void IrSender::trigger() {
//	HAL_TIM_PWM_Start(timer, dmaChannel);
	HAL_TIM_PWM_Start_DMA(timer, dmaChannel, (uint32_t*) pwmData, pwmSz);
	HAL_Delay(1337000);
}

void IrSender::stop() {
	HAL_TIM_PWM_Stop(timer, dmaChannel);
}

IrSender::~IrSender() {
//	HAL_TIM_PWM_Stop_DMA(timer, dmaChannel);
}

} /* namespace hitcon */
