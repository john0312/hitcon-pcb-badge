#ifndef HITCON_SERVICE_IR_SERVICE_H_
#define HITCON_SERVICE_IR_SERVICE_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>
#include <Logic/IrLogic.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>

namespace hitcon {

constexpr size_t IR_SERVICE_TX_SIZE = 32;
constexpr size_t IR_SERVICE_RX_SIZE = 32;
constexpr size_t IR_SERVICE_BUFFER_SIZE = PACKET_MAX_LEN;
constexpr uint16_t PWM_PULSE = 16;

struct IrBuffer {
	size_t size[2] {0, 0};
	uint32_t buffer[2][IR_SERVICE_BUFFER_SIZE];
};

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
  bool SendBuffer(uint8_t* data, size_t len);

  // Whenever IR_SERVICE_RX_SIZE bytes of bit array is received from IR,
  // the callback is called with uint8_t buffer[IR_SERVICE_RX_SIZE) of
  // data. After the callback is finished, the passed in buffer should be
  // considered invalid.
  void SetOnBufferReceived(callback_t callback, void* callback_arg1);

private:
  bool SendPacket(IrPacket &packet);
  void TransmitBuffer(const void *_slot);
  void TryQueueTransmitter(size_t slot);
  hitcon::service::sched::Task Transmitter;
  bool TransmitterActive;
  IrBuffer PwmBuffer;
  IrBuffer RxBuffer;
};

extern IrService irService;

}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_SERVICE_H_
