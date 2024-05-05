/*
 * IrSender.h
 *
 *  Created on: May 5, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_IRSENDER_H_
#define HITCON_IRSENDER_H_

#include <main.h>
#include <cstdint>

namespace hitcon {

class IrSender {
private:
	TIM_HandleTypeDef *timer;
	uint32_t dmaChannel;
public:
	IrSender();
	void init(TIM_HandleTypeDef *timer, uint32_t dmaChannel);
	void trigger();
	virtual ~IrSender();
};

} /* namespace hitcon */

#endif /* HITCON_IRSENDER_H_ */
