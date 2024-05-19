#ifndef HITCON_UTIL_CALLBACK_H_
#define HITCON_UTIL_CALLBACK_H_

namespace hitcon {

// Callback type for use in scheduler.
// 2 void ptr because we need to hold this for class method.
typedef void (*callback_t)(void*, void*);

}  // namespace hitcon

#endif  // #ifndef HITCON_UTIL_CALLBACK_H_
