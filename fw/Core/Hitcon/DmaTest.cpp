/*
 * DmaTest.cpp
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#include "DmaTest.h"
#include "main.h"

namespace hitcon {

DmaTest::DmaTest() {
	MakeBuf();
}

DmaTest::~DmaTest() {

}

void DmaTest::SetRow(unsigned char timestamp, unsigned char row) {
	const GpioBit &bit = DmaRowBits[row];
	bufs[bit.port][timestamp] |= 1 << bit.offset;
}

void DmaTest::ResetRow(unsigned char timestamp, unsigned char row) {
	const GpioBit &bit = DmaRowBits[row];
	bufs[bit.port][timestamp] |= 1 << (bit.offset + ResetOffset);
}

void DmaTest::SetCol(unsigned char timestamp, unsigned char col) {
	const GpioBit &bit = DmaColBits[col];
	bufs[bit.port][timestamp] |= 1 << bit.offset;
}

void DmaTest::ResetCol(unsigned char timestamp, unsigned char col) {
	const GpioBit &bit = DmaColBits[col];
	bufs[bit.port][timestamp] |= 1 << (bit.offset + ResetOffset);
}

void DmaTest::MakeBuf() {
	SetRow(0, 0);
	ResetRow(0, DmaCycleLen-1);
	for (unsigned char timestamp = 0; timestamp < DmaCycleLen; ++timestamp) {
		SetRow(timestamp, timestamp);
		ResetRow(timestamp, timestamp-1);
	}
}

static GPIO_TypeDef *cPorts[] = {LedCa_GPIO_Port, LedCb_GPIO_Port, LedCc_GPIO_Port, LedCd_GPIO_Port, LedCe_GPIO_Port, LedCf_GPIO_Port, LedCg_GPIO_Port, LedCh_GPIO_Port};
static GPIO_TypeDef *rPorts[] = {LedRa_GPIO_Port, LedRb_GPIO_Port, LedRc_GPIO_Port, LedRd_GPIO_Port, LedRe_GPIO_Port, LedRf_GPIO_Port, LedRg_GPIO_Port, LedRh_GPIO_Port};
static uint16_t cPins[] = {LedCa_Pin, LedCb_Pin, LedCc_Pin, LedCd_Pin, LedCe_Pin, LedCf_Pin, LedCg_Pin, LedCh_Pin};
static uint16_t rPins[] = {LedRa_Pin, LedRb_Pin, LedRc_Pin, LedRd_Pin, LedRe_Pin, LedRf_Pin, LedRg_Pin, LedRh_Pin};

void DmaTest::ResetBoard() {
	for (unsigned i = 0; i < DmaCSize; ++i)
		HAL_GPIO_WritePin(cPorts[i], cPins[i], GPIO_PIN_SET);
	for (unsigned i = 0; i < DmaRSize; ++i)
		HAL_GPIO_WritePin(rPorts[i], rPins[i], GPIO_PIN_SET);
}

void DmaTest::Trigger() {
	ResetBoard();
	for (unsigned char port = 0; port < DmaGpioPortCount; ++port) {
		DMA_HandleTypeDef *dmaChannel = dmaChannels[port];
		TIM_HandleTypeDef *timerHandler = timerHandlers[port];
		HAL_DMA_Start(dmaChannel, (uint32_t)(this->bufs[port]), (uint32_t)&(GpioPorts[port]->BSRR), DmaCycleLen);
		HAL_TIM_Base_Start(timerHandler);
		HAL_TIM_OC_Start(timerHandler, TIM_CHANNEL_1);
		timerHandler->Instance->DIER |= (1 << 8);
	}
}

void DmaTest::Init(DMA_HandleTypeDef *dmaChannels[DmaGpioPortCount], TIM_HandleTypeDef *timerHandlers[DmaGpioPortCount]) {
	for (unsigned char port = 0; port < DmaGpioPortCount; ++port) {
		this->dmaChannels[port] = dmaChannels[port];
		this->timerHandlers[port] = timerHandlers[port];
	}
}

} /* namespace hitcon */
