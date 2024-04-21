/*
 * DmaTest.cpp
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#include "DmaTest.h"
#include "main.h"

namespace hitcon {

static void memset(char *buf, char c, unsigned int sz) {
	for (unsigned int i = 0; i < sz; ++i)
		buf[i] = c;
}

DmaTest::DmaTest() {
	memset((char *)dmaBuf, 0, sizeof(dmaBuf));
	MakeBuf();
}

DmaTest::~DmaTest() {

}

inline GPIO_TypeDef *DmaTest::GetGpioPort() {
	return GPIOA;
}

void DmaTest::SetRow(unsigned char timestamp, unsigned char row) {
	const GpioBit &bit0 = DmaRowBits[0];
	const GpioBit &bit1 = DmaRowBits[1];
	const GpioBit &bit2 = DmaRowBits[2];
	if (row & 0b001)
		dmaBuf[timestamp] |= 1 << bit0.offset;
	else
		dmaBuf[timestamp] |= 1 << (bit0.offset + ResetOffset);
	if (row & 0b010)
		dmaBuf[timestamp] |= 1 << bit1.offset;
	else
		dmaBuf[timestamp] |= 1 << (bit1.offset + ResetOffset);
	if (row & 0b100)
		dmaBuf[timestamp] |= 1 << bit2.offset;
	else
		dmaBuf[timestamp] |= 1 << (bit2.offset + ResetOffset);
}

//void DmaTest::ResetRow(unsigned char timestamp, unsigned char row) {
//	const GpioBit &bit = DmaRowBits[row];
//	bufs[bit.port][timestamp] |= 1 << (bit.offset + ResetOffset);
//}

void DmaTest::SetCol(unsigned char timestamp, unsigned char col) {
	const GpioBit &bit = DmaColBits[col];
	dmaBuf[timestamp] |= 1 << bit.offset;
}

void DmaTest::ResetCol(unsigned char timestamp, unsigned char col) {
	const GpioBit &bit = DmaColBits[col];
	dmaBuf[timestamp] |= 1 << (bit.offset + ResetOffset);
}

void DmaTest::MakeBuf() {
	for (unsigned char timestamp = 0; timestamp < DmaCycleLen; ++timestamp) {
		SetRow(timestamp, timestamp);
		if (timestamp & 1) {
			SetCol(timestamp, 4);
			ResetCol(timestamp, 5);
			SetCol(timestamp, 6);
			ResetCol(timestamp, 7);
		}
		else {
			ResetCol(timestamp, 4);
			SetCol(timestamp, 5);
			ResetCol(timestamp, 6);
			SetCol(timestamp, 7);
		}
	}
}

static GPIO_TypeDef *cPorts[] = {LedCa_GPIO_Port, LedCb_GPIO_Port, LedCc_GPIO_Port, LedCd_GPIO_Port, LedCe_GPIO_Port, LedCf_GPIO_Port, LedCg_GPIO_Port, LedCh_GPIO_Port};
static GPIO_TypeDef *rPorts[] = {LedRa_GPIO_Port, LedRb_GPIO_Port, LedRc_GPIO_Port, LedRd_GPIO_Port, LedRe_GPIO_Port, LedRf_GPIO_Port, LedRg_GPIO_Port, LedRh_GPIO_Port};
static uint16_t cPins[] = {LedCa_Pin, LedCb_Pin, LedCc_Pin, LedCd_Pin, LedCe_Pin, LedCf_Pin, LedCg_Pin, LedCh_Pin};
static uint16_t rPins[] = {LedRa_Pin, LedRb_Pin, LedRc_Pin, LedRd_Pin, LedRe_Pin, LedRf_Pin, LedRg_Pin, LedRh_Pin};

void DmaTest::ResetBoard() {
	for (unsigned i = 0; i < DmaCSize; ++i)
		HAL_GPIO_WritePin(cPorts[i], cPins[i], GPIO_PIN_RESET);
	for (unsigned i = 0; i < 3; ++i)
		HAL_GPIO_WritePin(rPorts[i], rPins[i], GPIO_PIN_RESET);
}

void DmaTest::Trigger() {
	ResetBoard();
	HAL_DMA_Start(dmaChannel, (uint32_t)(dmaBuf), (uint32_t)&(GetGpioPort()->BSRR), DmaCycleLen);
	HAL_TIM_Base_Start(timerHandler);
	HAL_TIM_OC_Start(timerHandler, TIM_CHANNEL_ALL);
	timerHandler->Instance->DIER |= (1 << 8);
}

void DmaTest::Init(DMA_HandleTypeDef *dmaChannel, TIM_HandleTypeDef *timerHandler) {
	this->dmaChannel = dmaChannel;
	this->timerHandler = timerHandler;
}

} /* namespace hitcon */
