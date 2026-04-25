#ifndef HITCON_UTIL_INT_TO_STR_H_
#define HITCON_UTIL_INT_TO_STR_H_

#include <stdint.h>

namespace hitcon {

// Converts `n` to string and stores in `str` with a capacity of `size`.
// Returns the string length, excluding null terminator.
unsigned int uint_to_chr(char* str, unsigned int size, unsigned int n);
unsigned int uint_to_chr_hex(char* str, unsigned int size, unsigned int n);
char uint_to_chr_hex_nibble(uint8_t value);

}  // namespace hitcon

#endif  // #ifndef HITCON_UTIL_INT_TO_STR_H_
