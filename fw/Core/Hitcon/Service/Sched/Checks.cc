#include <Service/Sched/Checks.h>

#include "main.h"

namespace hitcon {
namespace service {
namespace sched {

#pragma GCC push_options
#pragma GCC optimize("O0")
void my_assert(bool expr) {
#ifdef ASSERTION_ENABLED
  if (!expr) {
    __disable_irq();
    for (int i = 0; i < 32; i++) {
      __NOP();
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
    unsigned x = 0U / 0U;  // Force a divide by zero to trigger a fault
#pragma GCC diagnostic pop
    (void)x;
    while (true) {
      __NOP();
    }
  }
#endif  // ASSERTION_ENABLED
};
#pragma GCC pop_options

void AssertOverflow() { my_assert(false); };

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
