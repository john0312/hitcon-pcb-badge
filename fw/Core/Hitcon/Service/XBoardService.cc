#include <Service/XBoardService.h>
#include <Service/Sched/Checks.h>

using namespace hitcon::service::sched;
using namespace hitcon::service::xboard;

namespace hitcon {
namespace service {
namespace xboard {

XBoardService g_xboard_service;

XBoardService::XBoardService()
    : _on_rx_callback(nullptr),
      _on_rx_callback_arg1(nullptr),
      _rx_task(480, (callback_t)&XBoardService::OnRxWrapper, this),
      _routine_task(490, (task_callback_t)&XBoardService::Routine, this, 10) {}

void XBoardService::Init() {
    scheduler.Queue(&_routine_task, nullptr);
    scheduler.EnablePeriodic(&_routine_task);

    TriggerRx();
}

void XBoardService::QueueDataForTx(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if ((_tx_buffer_tail + 1)%kTxBufferSize == _tx_buffer_head) {
            // Overflow, we're dropping data.
            AssertOverflow();
            break;
        }
        _tx_buffer[_tx_buffer_tail] = data[i];
        _tx_buffer_tail = (_tx_buffer_tail + 1) % kTxBufferSize;
    }
}

void XBoardService::SetOnByteRx(callback_t callback, void* callback_arg1) {
    _on_rx_callback = callback;
    _on_rx_callback_arg1 = callback_arg1;
}

void XBoardService::NotifyTxFinish() { _tx_busy = false; }

void XBoardService::NotifyRxFinish() {
    if (_rx_task_busy) {
        // Overflow, we dropped a byte.
    } else {
        if (_on_rx_callback) {
            _rx_task_busy = true;
            scheduler.Queue(&_rx_task, reinterpret_cast<void*>(
                                           static_cast<size_t>(rx_byte_)));
        }
    }
    TriggerRx();
}

// private function
void XBoardService::Routine(void*) {
    if (_tx_buffer_head != _tx_buffer_tail && !_tx_busy) {
        _tx_busy = true;
        HAL_UART_Transmit_IT(_huart, &_tx_buffer[_tx_buffer_head], 1);
        _tx_buffer_head = (_tx_buffer_head + 1) % kTxBufferSize;
    }
}

void XBoardService::TriggerRx() { HAL_UART_Receive_IT(_huart, &rx_byte_, 1); }

void XBoardService::OnRxWrapper(void* arg2) {
    _rx_task_busy = false;
    if (_on_rx_callback) {
        _on_rx_callback(_on_rx_callback_arg1, arg2);
    }
}

}  // namespace xboard
}  // namespace service
}  // namespace hitcon

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  g_xboard_service.NotifyTxFinish();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  g_xboard_service.NotifyRxFinish();
}
