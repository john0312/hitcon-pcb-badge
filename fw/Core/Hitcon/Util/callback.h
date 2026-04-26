#ifndef HITCON_UTIL_CALLBACK_H_
#define HITCON_UTIL_CALLBACK_H_

// This macro converts pmf (Pointer-to-Member-Function) to callback_t
// and doesn't generate compiler warning.
// NOTE: Only apply on
// - non-virtual method
// - non-multiple inheritance method
//
// Generate the same assembly code as (callback_t)(&cls::fn) for release build
// and only 2 more assembly instructions in debug build for internal variable
#define CB_CAST(pmf_expr)                            \
  ({                                                 \
    auto _tmp_pmf = (pmf_expr);                      \
    reinterpret_cast<hitcon::callback_t>(            \
        *reinterpret_cast<void** const>(&_tmp_pmf)); \
  })
/*
Difference between pmf and normal function pointer:
&cls::fn is a pointer to a struct like
struct PMF {
    uint32_t ptr; // first 4 bytes: address to function
    uint32_t adj; // last 4 bytes: `this` pointer offset
};
compiler complains when trying to interpret PMF like a normal 4 bytes
function pointer. use reinterpret_cast to bypass the type checking.
*/

namespace hitcon {

// Callback type for use in scheduler.
// 2 void ptr because we need to hold this for class method.
typedef void (*callback_t)(void*, void*);

}  // namespace hitcon

#endif  // #ifndef HITCON_UTIL_CALLBACK_H_
