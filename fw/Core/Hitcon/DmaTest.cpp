/*
 * DmaTest.cpp
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#include "DmaTest.h"
#include "main.h"

namespace hitcon {

static char src[10] = "deadbeef";
static char dst[10];

DmaTest::DmaTest(DMA_HandleTypeDef &dmaChannel) : dmaChannel(dmaChannel) {
	// TODO Auto-generated constructor stub
	UnsetBoard();
	// HAL_DMA_Start(&dmaChannel, src, dst, sizeof(src));
}

DmaTest::~DmaTest() {
	// TODO Auto-generated destructor stub

}

static GPIO_TypeDef *cPorts[] = {LedCa_GPIO_Port, LedCb_GPIO_Port, LedCc_GPIO_Port, LedCd_GPIO_Port, LedCe_GPIO_Port, LedCf_GPIO_Port, LedCg_GPIO_Port, LedCh_GPIO_Port};
static GPIO_TypeDef *rPorts[] = {LedRa_GPIO_Port, LedRb_GPIO_Port, LedRc_GPIO_Port, LedRd_GPIO_Port, LedRe_GPIO_Port, LedRf_GPIO_Port, LedRg_GPIO_Port, LedRh_GPIO_Port};
static uint16_t cPins[] = {LedCa_Pin, LedCb_Pin, LedCc_Pin, LedCd_Pin, LedCe_Pin, LedCf_Pin, LedCg_Pin, LedCh_Pin};
static uint16_t rPins[] = {LedRa_Pin, LedRb_Pin, LedRc_Pin, LedRd_Pin, LedRe_Pin, LedRf_Pin, LedRg_Pin, LedRh_Pin};

void DmaTest::SetBoard() {
	for (unsigned i = 0; i < cSize; ++i)
		HAL_GPIO_WritePin(cPorts[i], cPins[i], GPIO_PIN_SET);
	for (unsigned i = 0; i < rSize; ++i)
		HAL_GPIO_WritePin(rPorts[i], rPins[i], GPIO_PIN_SET);
}

void DmaTest::UnsetBoard() {
	for (unsigned i = 0; i < cSize; ++i)
		HAL_GPIO_WritePin(cPorts[i], cPins[i], GPIO_PIN_RESET);
	for (unsigned i = 0; i < rSize; ++i)
		HAL_GPIO_WritePin(rPorts[i], rPins[i], GPIO_PIN_RESET);
}

void DmaTest::Trigger() {
	// if (HAL_DMA_PollForTransfer())
	UnsetBoard();
	HAL_Delay(500);
	SetBoard();
	HAL_Delay(500);
}

} /* namespace hitcon */
