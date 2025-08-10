#include <Service/Sched/Checks.h>

namespace hitcon {
namespace service {
namespace sched {

#pragma GCC push_options
#pragma GCC optimize("O0")
void my_assert(bool expr) {
#ifdef DEBUG
  if (!expr) {
    unsigned x = 0U / 0U;  // Force a divide by zero to trigger a fault
  }
#endif  // DEBUG
};
#pragma GCC pop_options

void AssertOverflow() { my_assert(false); };

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
