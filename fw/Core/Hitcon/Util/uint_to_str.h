#ifndef HITCON_UTIL_INT_TO_STR_H_
#define HITCON_UTIL_INT_TO_STR_H_

#include <stdint.h>

namespace hitcon {

unsigned int uint_to_chr(char* str, unsigned int size, int n);
unsigned int uint_to_chr_hex(char* str, unsigned size, int n);
char uint_to_chr_hex_nibble(uint8_t value);

}  // namespace hitcon

#endif  // #ifndef HITCON_UTIL_INT_TO_STR_H_
