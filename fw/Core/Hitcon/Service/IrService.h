#ifndef HITCON_SERVICE_IR_SERVICE_H_
#define HITCON_SERVICE_IR_SERVICE_H_

#include <stddef.h>
#include <stdint.h>
#include <Util/callback.h>
#include <Service/Sched/PeriodicTask.h>
#include <Service/Sched/Scheduler.h>
#include <Service/IrParam.h>

namespace hitcon {
namespace ir {

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
  // This is an array of (byte) bool at 38kHz. Each byte represents 1 pulse at 38kHz.
  // Each byte must be 0 or 1.
  bool SendBuffer(uint8_t* data, size_t len);

  // Whenever IR_SERVICE_RX_SIZE bytes of bit array is received from IR,
  // the callback is called with uint8_t buffer[IR_SERVICE_RX_SIZE) of
  // data. After the callback is finished, the passed in buffer should be
  // considered invalid.
  void SetOnBufferReceived(callback_t callback, void* callback_arg1);

private:
  void TransmitBuffer(const void *_slot);
  void TryQueueTransmitter(size_t slot);
  hitcon::service::sched::Task Transmitter;
  bool TransmitterActive;
  IrBuffer PwmBuffer;
  IrBuffer RxBuffer;

  size_t CurrentSlot;

  // TODO: check if we need >1 callbacks
  // OnBufferReceived callback
  callback_t callback;
  void *callback_arg;
};

extern IrService irService;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_SERVICE_H_
