#ifndef HITCON_SERVICE_XBOARD_SERVICE_H_
#define HITCON_SERVICE_XBOARD_SERVICE_H_

#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

#include "Service/Sched/Scheduler.h"
#include "usart.h"

namespace hitcon {
namespace service {
namespace xboard {

class XBoardService {
 public:
  XBoardService();

  void Init();

  // Append the data for transmit.
  void QueueDataForTx(uint8_t* data, size_t len);

  // Whenever a byte is received, this will be called.
  void SetOnByteRx(callback_t callback, void* callback_arg1);

  // to be called by interrupt function
  void NotifyTxFinish();

  // to be called by interrupt function
  void NotifyRxFinish();

  // 48 * 3 = 144, 3 packets
  static constexpr size_t kTxBufferSize = 160;

  UART_HandleTypeDef* _huart = &huart2;

  uint32_t sr_accu = 0;
  uint32_t sr_clear = 0;

 private:
  void Routine(void*);

  void TriggerRx();

  void OnRxWrapper(void* arg2);

  callback_t _on_rx_callback;
  void* _on_rx_callback_arg1;
  bool _tx_busy = false;
  bool _rx_task_busy = false;
  hitcon::service::sched::Task _rx_task;
  hitcon::service::sched::PeriodicTask _routine_task;

  uint8_t rx_byte_;
  uint8_t _tx_buffer[kTxBufferSize];
  // Next byte to be written to hardware.
  int _tx_buffer_head = 0;
  // Next byte from the upper layer.
  int _tx_buffer_tail = 0;

  // This is a huge hack. rx might get stopped so we want to restart it.
  int _rx_stopped_count = 0;
};

extern XBoardService g_xboard_service;

}  // namespace xboard
}  // namespace service
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_XBOARD_SERVICE_H_
