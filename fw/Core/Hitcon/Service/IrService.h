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

  // Return true if the IrService is free to send a buffer now.
  bool CanSendBufferNow();

  // Call to send an IR packet.
  // This is a packed bit array, each bit is PULSE_PER_DATA_BIT pulse at 38kHz.
  // The least significant bit of a byte is the first transmitted bit.
  // Caller must guarantee that the buffer is valid and not changed during the
  // whole transmission process.
  // If send_header is true, we'll prepend the header during transmission.
  bool SendBuffer(const uint8_t* data, size_t len, bool send_header);

  // Whenever we've collected of IR_SERVICE_RX_ON_BUFFER_SIZE bytes of receive
  // buffer, we'll call the specified function.
  // The callback is called with a uint8_t pointer to an array of size IR_SERVICE_RX_ON_BUFFER_SIZE bytes. Each byte in the array is 8 sample points.
  // Each of the sample point is equivalent to 4 pulse at 38kHz.
  // After the callback is finished, the passed in buffer should be
  // considered invalid.
  void SetOnBufferReceived(callback_t callback, void* callback_arg1);

  uint16_t rx_dma_buffer[2*IR_SERVICE_RX_SIZE];
  uint16_t tx_dma_buffer[2*IR_SERVICE_TX_SIZE];
  uint8_t rx_buffer[2*IR_SERVICE_RX_ON_BUFFER_SIZE];
  size_t rx_buffer_base;

  uint8_t calllback_pass_arr[IR_SERVICE_RX_SIZE];

  callback_t on_rx_buffer_cb;
  void *on_rx_buffer_arg;

  // Need to be public to be queued by the callback.
  hitcon::service::sched::Task dma_tx_populate_task;

  // Need to be public to be queued by the callback.
  hitcon::service::sched::Task dma_rx_pull_task;

private:
  const uint8_t* tx_pending_buffer;
  uint32_t tx_pending_buffer_len;
  // If false, will skip sending header.
  bool tx_pending_send_header;

  /*
  MSB meaning:
  0x00 - Ready to accept the next buffer.
  0x01 - Waiting for air space to silent.
  0x02 - Waiting for collision to finish.
  0x03 - Sending header.
  0x04 - Sending body.
  */
  uint32_t tx_state;

  // How long has rx been quiet?
  // This is in units of 8/DECODE_SAMPLE_RATIO bit time.
  size_t rx_quiet_cnt;

  // How long should we wait until we transmit?
  size_t rx_required_quiet_period;

  // How many RX DMA Run since the tx is released?
  size_t rx_ctr_since_release;

  hitcon::service::sched::PeriodicTask routine_task;

  hitcon::service::sched::Task on_rx_callback_runner;

  bool rx_on_buffer_callback_finished;

  // Call to populate TX DMA Buffer.
  // ptr_side is to be reinterpret_cast<int>(), and will be 0 or 1.
  void PopulateTxDmaBuffer(void* ptr_side);

  // Call to pull RX DMA Buffer.
  // ptr_side is to be reinterpret_cast<int>(), and will be 0 or 1.
  void PullRxDmaBuffer(void* ptr_side);

  // A slow routine function to keep track of states.
  void Routine(void* arg1);

  // Just a wrapper for calling on_rx_buffer_cb.
  void OnBufferRecvWrapper(void* arg2);
};

extern IrService irService;

}  // namespace ir
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_IR_SERVICE_H_
