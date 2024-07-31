#include <Service/NoiseSource.h>

#include "adc.h"

using namespace hitcon::service::sched;

namespace hitcon {
NoiseSource g_noise_source;

NoiseSource::NoiseSource()
    : _routine_task(810, (task_callback_t)&NoiseSource::Routine, this,
                    kRoutinePeriod / kNoiseLen),
      _cb_task(805, (task_callback_t)&NoiseSource::CallbackWrapper, this),
      on_noise_cb(nullptr) {}

void AdcCpltCallback(ADC_HandleTypeDef* hadc) {
  if (hadc == &hadc1) {
    scheduler.Queue(&g_noise_source._cb_task, nullptr);
  }
}

void NoiseSource::Init() {
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  hadc1.ConvCpltCallback = &AdcCpltCallback;
}

void NoiseSource::Routine(void* unused) { HAL_ADC_Start_IT(&hadc1); }

void NoiseSource::CallbackWrapper(void* unused) {
  adc_values[current_index] = HAL_ADC_GetValue(&hadc1);
  current_index++;
  if (current_index == kNoiseLen) {
    current_index = 0;
    if (on_noise_cb) on_noise_cb(on_noise_cb_arg, adc_values);
  }
}

}  // namespace hitcon
