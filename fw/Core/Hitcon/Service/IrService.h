#ifndef HITCON_SERVICE_IR_SERVICE_H_
#define HITCON_SERVICE_IR_SERVICE_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>

namespace hitcon {

constexpr size_t IR_SERVICE_TX_SIZE = 32;
constexpr size_t IR_SERVICE_RX_SIZE = 32;

class IrService {
 public:
  IrService();

  // Init should:
  // - Setup TIM3 to run at 38kHz.
  // - Setup TIM3_CH3 to output PWM.
  // - Setup DMA1 CH2 to read from IrService's pwm_buffer.
  //   (Double buffering should be used)
  // - Setup TIM2 to run at 9.5kHz.
  // - Setup TIM2 CH3.
  // - Setup DMA1 CH1 to read PA on every TIM2 CH3 event.
  //   (Double buffering should be used)
  void Init();

  // Call to send an IR packet.
  // This is a bit stream at 38kHz. Each bit represents 1 pulse at 38kHz.
  void TransmitBuffer(uint8_t* packet, size_t packet_len);

  // Whenever IR_SERVICE_RX_SIZE bytes of bit array is received from IR,
  // the callback is called with uint8_t buffer[IR_SERVICE_RX_SIZE) of
  // data. After the callback is finished, the passed in buffer should be
  // considered invalid.
  void SetOnBufferReceived(callback_t callback, void* callback_arg1);

private:
  uint16_t pwm_buffer_[IR_SERVICE_TX_SIZE*2];
  uint16_t rx_buffer_[IR_SERVICE_RX_SIZE*2];
};

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_SERVICE_H_
