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
namespace ir {

IrService irService;

IrService::IrService() : Transmitter(100, (task_callback_t)&IrService::TransmitBuffer, this), TransmitterActive(false), CurrentSlot(0) {
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

bool IrService::SendBuffer(uint8_t* data, size_t len) {
    if (PwmBuffer.size[CurrentSlot])
		return false;

	for (size_t i = 0; i < len; ++i)
		PwmBuffer.buffer[CurrentSlot][i] = PWM_PULSE * data[i];

	TryQueueTransmitter(CurrentSlot);
	CurrentSlot = !CurrentSlot;
	return true;
}

void IrService::SetOnBufferReceived(callback_t callback, void* callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

}  // namespace ir
}  // namespace hitcon
