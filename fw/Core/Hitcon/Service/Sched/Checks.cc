#include <Service/Sched/Checks.h>

namespace hitcon {
namespace service {
namespace sched {

void my_assert(bool expr) {
#ifdef DEBUG
  if (!expr) {
    ((char*)nullptr)[0] = 0;
  }
#endif  // DEBUG
};

void AssertOverflow() { my_assert(false); };

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
