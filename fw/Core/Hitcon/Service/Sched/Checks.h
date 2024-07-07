
namespace hitcon {
namespace service {
namespace sched {

void my_assert(bool expr);

// This is used to alert developers of any overflow conditions.
// Call this whenever there's overflow or underflow.
void AssertOverflow();

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */
