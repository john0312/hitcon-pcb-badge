#ifndef SERVICE_NOISE_SOURCE_DOT_H
#define SERVICE_NOISE_SOURCE_DOT_H

#include <Service/Sched/Scheduler.h>

namespace hitcon {

class NoiseSource {
private:
 void Routine(void* unused);
 void CallbackWrapper(void* unused);

 hitcon::service::sched::PeriodicTask _routine_task;
 hitcon::service::sched::task_callback_t on_noise_cb;
 void* on_noise_cb_arg;

 // three adc channels: noise_in, temperature, Vrefint
 static constexpr size_t kChannelAmount = 3;
 static constexpr size_t kNoiseLen = 2 * kChannelAmount;
 // interval between each on_noise_cb is called
 static constexpr size_t kRoutinePeriod = 60000; // 1 min
 uint16_t adc_values[kNoiseLen];
 size_t current_index = 0;
 public:
  NoiseSource();

  void Init();

  // Whenever we collected kNoiseLen of noise, we'll call the callback.
  void SetOnNoiseBytes(hitcon::service::sched::task_callback_t callback,
		  void* callback_arg1) {
    on_noise_cb = callback;
    on_noise_cb_arg = callback_arg1;
  }

  hitcon::service::sched::Task _cb_task;
};

extern NoiseSource g_noise_source;
}  // namespace hitcon

#endif  // SERVICE_NOISE_SOURCE_DOT_H
