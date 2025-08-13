#include <Logic/ButtonLogic.h>
#include <Service/ButtonService.h>
#include <Service/Sched/Scheduler.h>
#include <main.h>

#include <cstdint>
using namespace hitcon::service::sched;
namespace hitcon {
ButtonLogic g_button_logic;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
ButtonLogic::ButtonLogic()
    : _callback_task(642, (callback_t)&ButtonLogic::CallbackWrapper, this),
      _edge_callback_task(643, (callback_t)&ButtonLogic::EdgeCallbackWrapper,
                          this) {}
#pragma GCC diagnostic pop

void ButtonLogic::Init() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_button_service.SetDataInCallback((callback_t)&ButtonLogic::OnReceiveData,
                                     this);
#pragma GCC diagnostic pop
  for (uint8_t i = 0; i < BUTTON_AMOUNT; i++) {
    _count[i] = 0;
    _edge_flag[i] = 0;
  }

  _fire = 0;
}

void ButtonLogic::SetCallback(callback_t callback, void* callback_arg1) {
  this->callback = callback;
  this->callback_arg1 = callback_arg1;
}

void ButtonLogic::SetEdgeCallback(callback_t callback, void* callback_arg1) {
  this->edge_callback = callback;
  this->edge_callback_arg1 = callback_arg1;
}

void ButtonLogic::CallbackWrapper(void* unused) {
  _is_queued = _is_queued & (~0x01);
  if (_btn_queue.Size() != 0) {
    if (callback) {
      callback(callback_arg1, reinterpret_cast<void*>(_btn_queue.Front()));
    }
    _btn_queue.PopFront();
    EnsureBtnQueued();
  }
}

void ButtonLogic::EdgeCallbackWrapper(void* arg2) {
  _is_queued = _is_queued & (~0x02);
  if (_edge_queue.Size() != 0) {
    if (edge_callback) {
      edge_callback(edge_callback_arg1,
                    reinterpret_cast<void*>(_edge_queue.Front()));
    }
    _edge_queue.PopFront();
    EnsureEdgeQueued();
  }
}

int q_cnt1 = 0;
int q_cnt2 = 0;

void ButtonLogic::EnsureBtnQueued() {
  if ((_is_queued & 0x01) == 0 && _edge_queue.Size() != 0) {
    _is_queued = _is_queued | 0x01;
    q_cnt1++;
    if (q_cnt1 == 0) {
    }
    scheduler.Queue(&_callback_task, nullptr);
  }
}

void ButtonLogic::EnsureEdgeQueued() {
  if ((_is_queued & 0x02) == 0 && _btn_queue.Size() != 0) {
    _is_queued = _is_queued | 0x02;
    q_cnt2++;
    scheduler.Queue(&_edge_callback_task, nullptr);
  }
}
void ButtonLogic::OnReceiveData(uint8_t* data) {
  static uint8_t counter = 0;
  uint16_t pressed_btn = 0;
  for (uint8_t i = 0; i < kDatasetSize; i++) {
    for (uint8_t j = 0; j < BUTTON_AMOUNT; j++) {
      if (((data[i] >> (j)) & 1) == 1) {
        _count[j]++;
        pressed_btn = j;
        if (_edge_flag[j] == 0 && _count[j] > BOUNCE_TIME_THRESHOLD) {
          _edge_flag[j] = 1;
          _edge_queue.PushBack(
              static_cast<size_t>(((BUTTON_MODE + j) | BUTTON_KEYDOWN_BIT)));
          EnsureEdgeQueued();
        }
      } else {
        if (_fire == j) _fire = 0;
        if (BOUNCE_TIME_THRESHOLD < _count[j] &&
            _count[j] <= LONG_PRESS_TIME_THRESHOLD) {  // handle short press
          _out = BUTTON_MODE + j;
          _btn_queue.PushBack(static_cast<size_t>((_out)));
          EnsureBtnQueued();
        } else if (LONG_PRESS_TIME_THRESHOLD < _count[j]) {
          _out = BUTTON_LONG_MODE + j;
          _btn_queue.PushBack(static_cast<size_t>((_out)));
          EnsureBtnQueued();
        }
        _count[j] = 0;
        if (_edge_flag[j]) {
          _edge_flag[j] = 0;
          _edge_queue.PushBack(
              static_cast<size_t>(((BUTTON_MODE + j) | BUTTON_KEYUP_BIT)));
          EnsureEdgeQueued();
        }
      }
    }
  }
  if (_fire != 0) {
    if (counter == 5) {
      counter = 0;
      _out = BUTTON_MODE + _fire;
      _btn_queue.PushBack(static_cast<size_t>((_out)));
      EnsureBtnQueued();
    }
    counter++;
  }

  if (_count[pressed_btn] > 150) {
    _fire = pressed_btn;
  }
}

}  // namespace hitcon
