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
#include "IrLib.h"

namespace hitcon {

class IrSender {
private:
	TIM_HandleTypeDef *timer;
	uint32_t dmaChannel;
	raw_packet_t sendingPacket;
public:
	IrSender();
	void init(TIM_HandleTypeDef *timer, uint32_t dmaChannel);
	void send(uint32_t size, uint8_t *data);
	void trigger();
	void stop();
	virtual ~IrSender();
};

} /* namespace hitcon */

#endif /* HITCON_IRSENDER_H_ */
