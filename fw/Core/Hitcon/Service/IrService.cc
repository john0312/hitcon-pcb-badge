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
    : dma_tx_populate_task(
          100, (task_callback_t)&IrService::PopulateTxDmaBuffer, this),
      on_buf_recv_task(300, (callback_t)&IrService::OnBufferRecvWrapper, this),
      routine_task(600, (callback_t)&IrService::Routine, this, 22) {}

void ReceiveDmaHalfCplt(DMA_HandleTypeDef *hdma) {
  for (uint8_t i = 0; i < IR_SERVICE_RX_SIZE; i++)
    irService.calllback_pass_arr[i] = irService.rx_dma_buffer[i] & IrRx_Pin;
  //    scheduler.Queue(&irService.on_buf_recv_task,
  //		    (void*)irService.calllback_pass_arr);
  irService.callback(irService.callback_arg,
                     (void *)&(irService.calllback_pass_arr));
}

void ReceiveDmaCplt(DMA_HandleTypeDef *hdma) {
  for (uint8_t i = 0; i < IR_SERVICE_RX_SIZE; i++)
    irService.calllback_pass_arr[i] =
        irService.rx_dma_buffer[i + IR_SERVICE_RX_SIZE] & IrRx_Pin;
  //  scheduler.Queue(&irService.on_buf_recv_task,
  //		    (void*)irService.calllback_pass_arr);
  irService.callback(irService.callback_arg,
                     (void *)&irService.calllback_pass_arr);
}

void TransmitDmaHalfCplt(DMA_HandleTypeDef *hdma) {
  scheduler.Queue(&irService.dma_tx_populate_task, reinterpret_cast<void *>(0));
}

void TransmitDmaCplt(DMA_HandleTypeDef *hdma) {
  scheduler.Queue(&irService.dma_tx_populate_task, reinterpret_cast<void *>(1));
}

void IrService::OnBufferRecvWrapper(void *arg2) {
  if (callback) callback(callback_arg, arg2);
}

void IrService::Init() {
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  hdma_tim2_ch3.XferHalfCpltCallback = &ReceiveDmaHalfCplt;
  hdma_tim2_ch3.XferCpltCallback = &ReceiveDmaCplt;
  hdma_tim3_ch3.XferCpltCallback = &TransmitDmaCplt;
  hdma_tim3_ch3.XferHalfCpltCallback = &TransmitDmaHalfCplt;
  HAL_DMA_Start_IT(&hdma_tim2_ch3, reinterpret_cast<uint32_t>(&GPIOA->IDR),
                   reinterpret_cast<uint32_t>(rx_dma_buffer),
                   IR_SERVICE_RX_SIZE * 2);
  HAL_DMA_Start_IT(&hdma_tim3_ch3, reinterpret_cast<uint32_t>(tx_dma_buffer),
                   reinterpret_cast<uint32_t>(&(htim3.Instance->CCR3)),
                   IR_SERVICE_TX_SIZE * 2);
}

bool IrService::CanSendBufferNow() { return tx_state == 0x00000000; }

bool IrService::SendBuffer(uint8_t *data, size_t len, bool send_header) {
  if (tx_state != 0x00000000) {
    // Can't send buffer now, we're handling another buffer.
    return false;
  }

  tx_pending_buffer = data;
  tx_pending_buffer_len = len;
  tx_pending_send_header = send_header;

  tx_state = 0x01000000;

  return true;
}

void IrService::SetOnBufferReceived(callback_t callback, void *callback_arg1) {
  this->callback = callback;
  this->callback_arg = callback_arg1;
}

void IrService::PopulateTxDmaBuffer(void *ptr_side) {
  int side = reinterpret_cast<intptr_t>(ptr_side);

  uint32_t cstate = tx_state >> 24;
  int dma_base = (-side) & static_cast<int>(IR_SERVICE_TX_SIZE / 2);
  if (cstate <= 2) {
    // Not transmitting.
    for (int i = 0; i < IR_SERVICE_TX_SIZE / 2; i++) {
      tx_dma_buffer[dma_base + i] = 0;
    }
  } else if (cstate == 3) {
    // Send header.
    size_t ctr = tx_state & 0x00FFFFFF;
    size_t base = ctr * IR_PACKET_PER_RUN;
    for (int i = 0; i < IR_SERVICE_TX_SIZE / 2; i++) {
      tx_dma_buffer[dma_base + i] =
          (-static_cast<int16_t>(IR_PACKET_HEADER[base + i / 8])) &
          IR_PWM_TIM_CCR;
    }
    tx_state++;
    if (ctr == IR_PACKET_RUN_COUNT - 1) {
      // Done.
      tx_state = 0x04000000;
    }
  } else if (cstate == 4) {
    // Send body.
    size_t ctr = tx_state & 0x00FFFFFF;
    size_t base_bit = ctr * IR_BITS_PER_TX_RUN;
    int i = 0;
    for (int j = 0; j < IR_BITS_PER_TX_RUN; j++) {
      int cbit = (tx_pending_buffer[base_bit / 8] >> (base_bit % 8)) & 0x01;
      int16_t ccr_val = (-static_cast<int16_t>(cbit)) & IR_PWM_TIM_CCR;
      for (int k = 0; k < PULSE_PER_DATA_BIT; k++, i++) {
        tx_dma_buffer[i] = ccr_val;
      }
    }
    tx_state++;
    if (ctr + 1 == (tx_pending_buffer_len * 8 / IR_BITS_PER_TX_RUN)) {
      // Transmission done.
      tx_state = 0x00000000;
    }
  } else {
    my_assert(false);
  }
}

void IrService::Routine(void *arg1) {
  uint32_t tx_cstate = tx_state >> 24;
  if (tx_cstate == 0x01) {
    tx_state++;
    size_t ctr = tx_state & 0x00FFFFFF;
    if (ctr == 64) {
      tx_state = 0x03000000;
    }
  }
}

}  // namespace ir
}  // namespace hitcon
