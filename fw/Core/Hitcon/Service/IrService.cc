#include <Service/IrService.h>
#include <main.h>
#include <tim.h>

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim != &htim3)
		return;
	HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_3);
}

using namespace hitcon::service::sched;

namespace hitcon {

IrService irService;
IrLogic IrLogic;

IrService::IrService() : Transmitter(100, (task_callback_t)&IrService::TransmitBuffer, this), TransmitterActive(false) {
}

void IrService::TransmitBuffer(const void *_slot) {
	const size_t slot = (size_t)_slot;
	if (!PwmBuffer.size[slot]) {
		TransmitterActive = false;
		return;
	}
	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3, PwmBuffer.buffer[slot], PwmBuffer.size[slot]);
	TryQueueTransmitter(!slot);
}

void IrService::Init() {
  // TODO
}

void unpackPacket(uint8_t *packet, size_t packet_len) {
  // TODO
}

void IrService::TryQueueTransmitter(size_t slot) {
	if (TransmitterActive)
		return;
	scheduler.Queue(&Transmitter, (void *)slot);
}

bool IrService::SendPacket(IrPacket &packet) {
	static size_t currentSlot = 0;

	if (PwmBuffer.size[currentSlot])
		return false;

	uint8_t *data = packet.data();
	uint8_t size = packet.size();
	for (size_t i = 0; i < size; ++i)
		PwmBuffer.buffer[currentSlot][i] = PWM_PULSE * data[i];

	TryQueueTransmitter(currentSlot);
	currentSlot = !currentSlot;
	return true;
}

bool IrService::SendBuffer(uint8_t* data, size_t len) {
	IrPacket packet;
	IrLogic.EncodePacket(data, len, packet);

	return SendPacket(packet);
}

void IrService::SetOnBufferReceived(callback_t callback, void* callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

}  // namespace hitcon
