#include <Service/Sched/Checks.h>

#include "main.h"

namespace hitcon {
namespace service {
namespace sched {

#pragma GCC push_options
#pragma GCC optimize("O0")
void my_assert(bool expr) {
#ifdef DEBUG
  if (!expr) {
    __disable_irq();
    for (int i = 0; i < 32; i++) {
      __NOP();
    }
    unsigned x = 0U / 0U;  // Force a divide by zero to trigger a fault
    (void)x;
    while (true) {
      __NOP();
    }
  }
#endif  // DEBUG
};
#pragma GCC pop_options

void AssertOverflow() { my_assert(false); };

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
