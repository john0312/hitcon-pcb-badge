#ifndef SERVICE_NOISE_SOURCE_DOT_H
#define SERVICE_NOISE_SOURCE_DOT_H

#include <Service/Sched/Scheduler.h>

namespace hitcon {

class NoiseSource {
 public:
  NoiseSource();

  void Init();

  // Whenever we collected kNoiseLen of noise, we'll call the callback.
  void SetOnNoiseBytes(hitcon::service::sched::task_callback_t callback,
		  void* callback_arg1) {
    on_noise_cb = callback;
    on_noise_cb_arg = callback_arg1;
  }

 private:
  hitcon::service::sched::task_callback_t on_noise_cb;
  void* on_noise_cb_arg;

  static constexpr size_t kNoiseLen = 2;
};

}  // namespace hitcon

#endif  // SERVICE_NOISE_SOURCE_DOT_H
