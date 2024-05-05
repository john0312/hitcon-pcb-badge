/*
 * IrSender.cpp
 *
 *  Created on: May 5, 2024
 *      Author: aoaaceai
 */

#include "IrSender.h"
#include "main.h"

namespace hitcon {

const uint32_t pwmSz = 256;

uint32_t pwmData[pwmSz];

IrSender::IrSender() {

}

void IrSender::init(TIM_HandleTypeDef *timer, uint32_t dmaChannel) {
	this->timer = timer;
	this->dmaChannel = dmaChannel;
	for (unsigned i = 0; i < pwmSz; ++i)
		pwmData[i] = i;
}

void IrSender::trigger() {
	HAL_TIM_PWM_Start_DMA(timer, dmaChannel, pwmData, pwmSz);
}

IrSender::~IrSender() {
	HAL_TIM_PWM_Stop_DMA(timer, dmaChannel);
}

} /* namespace hitcon */
