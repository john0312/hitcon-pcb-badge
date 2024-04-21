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
	void Init(DMA_HandleTypeDef *dmaChannel, TIM_HandleTypeDef *timerHandler);
	void MakeBuf();
private:
	uint32_t dmaBuf[DmaCycleLen];
	DMA_HandleTypeDef *dmaChannel;
	TIM_HandleTypeDef *timerHandler;
	void ResetBoard();
	void SetRow(unsigned char timestamp, unsigned char row);
	void SetCol(unsigned char timestamp, unsigned char col);
	void ResetCol(unsigned char timestamp, unsigned char col);
	inline GPIO_TypeDef *GetGpioPort();
};



constexpr GpioBit DmaRowBits[DmaRSize] = {
	{10},
	{9},
	{8},
	{15},
	{15},
	{15},
	{15},
	{15}
};
constexpr GpioBit DmaColBits[DmaCSize] = {
	{15},
	{15},
	{15},
	{15},
	{7},
	{6},
	{5},
	{4}
};

} /* namespace hitcon */

#ifdef __cplusplus
}
#endif

#endif /* HITCON_DMATEST_H_ */
