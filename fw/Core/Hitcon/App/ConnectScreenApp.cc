#include "ConnectScreenApp.h"

#include <Logic/BaseStnHub.h>
#include <Logic/Display/display.h>
#include <Service/Sched/Scheduler.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::basestn;

namespace hitcon {
namespace app {
namespace connect_screen {

ConnectScreen::ConnectScreen()
    : _routine_task(490, (task_callback_t)&ConnectScreen::UpdateDisplay, this,
                    1000) {}

void ConnectScreen::OnEntry() {
  scheduler.Queue(&_routine_task, nullptr);
  scheduler.EnablePeriodic(&_routine_task);
  UpdateDisplay();
}

void ConnectScreen::OnExit() { scheduler.DisablePeriodic(&_routine_task); }

void ConnectScreen::OnButton(button_t button) {}

void ConnectScreen::UpdateDisplay() {
  // Get XBoard RX metrics from BaseStnHub
  const BufferMetric& xbrx_metric =
      g_basestn_hub.GetBufferMetric(BufferType::XBRX);

  // Create display string with only 3 digits (mod 10)
  char display_str[4];  // 3 digits + null terminator

  // Get last digit of each metric (mod 10)
  display_str[0] = '0' + (xbrx_metric.put % 10);
  display_str[1] = '0' + (xbrx_metric.taken % 10);
  display_str[2] = '0' + (xbrx_metric.dropped % 10);
  display_str[3] = '\0';

  // Display the three digits using fixed text mode
  display_set_mode_text(display_str);
}

ConnectScreen connect_screen;

}  // namespace connect_screen
}  // namespace app
}  // namespace hitcon