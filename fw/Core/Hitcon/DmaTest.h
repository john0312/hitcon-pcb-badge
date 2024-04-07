/*
 * DmaTest.h
 *
 *  Created on: Apr 7, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_DMATEST_H_
#define HITCON_DMATEST_H_

#include "main.h"

namespace hitcon {

class DmaTest {
public:
	DmaTest(DMA_HandleTypeDef &dmaChannel);
	virtual ~DmaTest();
	void Trigger();
	const unsigned rSize = 8;
	const unsigned cSize = 8;
private:
	DMA_HandleTypeDef &dmaChannel;
	void SetBoard();
	void UnsetBoard();
};

} /* namespace hitcon */

#endif /* HITCON_DMATEST_H_ */
