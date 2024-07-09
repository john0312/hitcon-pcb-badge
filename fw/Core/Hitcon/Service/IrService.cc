#include <Service/IrService.h>
#include <main.h>
#include <tim.h>

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  if (htim != &htim3) return;
  HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_3);
}

using namespace hitcon::service::sched;

namespace hitcon {
namespace ir {

IrService irService;

IrService::IrService()
    : Transmitter(100, (task_callback_t)&IrService::TransmitBuffer, this),
      TransmitterActive(false), CurrentSlot(0),
      on_buf_recv_task(63, (callback_t)&IrService::OnBufferRecvWrapper, this) {}

void IrService::TransmitBuffer(const void *_slot) {
  const size_t slot = (size_t)_slot;
  if (!PwmBuffer.size[slot]) {
    TransmitterActive = false;
    return;
  }
  HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3, PwmBuffer.buffer[slot],
                        PwmBuffer.size[slot]);
  TryQueueTransmitter(!slot);
}

void TransferHalfComplete(DMA_HandleTypeDef *hdma) {
  for (uint8_t i = 0; i < IR_SERVICE_RX_SIZE; i++)
    irService.calllback_pass_arr[i] = irService.dma_buffer[i] & IrRx_Pin;
//    scheduler.Queue(&irService.on_buf_recv_task,
//		    (void*)irService.calllback_pass_arr);
  irService.callback(irService.callback_arg,
                     (void *)&(irService.calllback_pass_arr));
}

void TransferComplete(DMA_HandleTypeDef *hdma) {
  for (uint8_t i = 0; i < IR_SERVICE_RX_SIZE; i++)
    irService.calllback_pass_arr[i] =
        irService.dma_buffer[i + IR_SERVICE_RX_SIZE] & IrRx_Pin;
//  scheduler.Queue(&irService.on_buf_recv_task,
//		    (void*)irService.calllback_pass_arr);
  irService.callback(irService.callback_arg,
                     (void *)&irService.calllback_pass_arr);
}

void IrService::OnBufferRecvWrapper(void *arg2) {
  if (callback) callback(callback_arg, arg2);
}

void IrService::Init() {
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  hdma_tim2_ch3.XferHalfCpltCallback = &TransferHalfComplete;
  hdma_tim2_ch3.XferCpltCallback = &TransferComplete;
  HAL_DMA_Start_IT(&hdma_tim2_ch3, (uint32_t)&GPIOA->IDR, (uint32_t)dma_buffer,
                   IR_SERVICE_RX_SIZE * 2);
}

void unpackPacket(uint8_t *packet, size_t packet_len) {
  // TODO
}

void IrService::TryQueueTransmitter(size_t slot) {
  if (TransmitterActive) return;
  scheduler.Queue(&Transmitter, (void *)slot);
}

bool IrService::SendBuffer(uint8_t *data, size_t len) {
  if (PwmBuffer.size[CurrentSlot]) return false;

  for (size_t i = 0; i < len; ++i)
    PwmBuffer.buffer[CurrentSlot][i] = PWM_PULSE * data[i];

  TryQueueTransmitter(CurrentSlot);
  CurrentSlot = !CurrentSlot;
  return true;
}

void IrService::SetOnBufferReceived(callback_t callback, void *callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

}  // namespace ir
}  // namespace hitcon
