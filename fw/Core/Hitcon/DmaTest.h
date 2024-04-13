/*
 * DmaTest.h
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_DMATEST_H_
#define HITCON_DMATEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

namespace hitcon {

struct GpioBit {
	unsigned char port;
	unsigned char offset;
};

constexpr unsigned char ResetOffset = 16;
constexpr unsigned char DmaRSize = 8;
constexpr unsigned char DmaCSize = 8;
constexpr unsigned char DmaCycleLen = 8;
constexpr unsigned char DmaGpioPortCount = 2;

class DmaTest {
public:
	DmaTest();
	virtual ~DmaTest();
	void Trigger();
	void Init(DMA_HandleTypeDef *dmaChannels[DmaGpioPortCount], TIM_HandleTypeDef *timerHandlers[DmaGpioPortCount]);
	void MakeBuf();
private:
	uint32_t bufs[DmaGpioPortCount][DmaCycleLen];
	DMA_HandleTypeDef *dmaChannels[DmaGpioPortCount];
	TIM_HandleTypeDef *timerHandlers[DmaGpioPortCount];
	void ResetBoard();
	void SetRow(unsigned char timestamp, unsigned char row);
	void ResetRow(unsigned char timestamp, unsigned char row);
	void SetCol(unsigned char timestamp, unsigned char col);
	void ResetCol(unsigned char timestamp, unsigned char col);
};

constexpr GPIO_TypeDef *GpioPorts[DmaGpioPortCount] = {GPIOA, GPIOB};

constexpr GpioBit DmaRowBits[DmaRSize] = {
	{0, 10},
	{0, 9},
	{0, 8},
	{1, 15},
	{1, 14},
	{1, 13},
	{1, 12},
	{1, 11}
};
constexpr GpioBit DmaColBits[DmaCSize] = {
	{1, 10},
	{1, 2},
	{1, 1},
	{1, 0},
	{0, 7},
	{0, 6},
	{0, 5},
	{0, 4}
};

} /* namespace hitcon */

#ifdef __cplusplus
}
#endif

#endif /* HITCON_DMATEST_H_ */
