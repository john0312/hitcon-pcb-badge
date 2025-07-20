#include "uint_to_str.h"

#include <string.h>

unsigned int hitcon::uint_to_chr(char *str, unsigned size, int n) {
  int count = 0;
  do {
    str[count++] = '0' + (n % 10);
    n /= 10;
  } while ((n != 0) && (count < size - 1));
  str[count] = 0;
  // reverse string
  char *p1 = str;
  char *p2 = str + strlen(p1) - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1++ = *p2;
    *p2-- = tmp;
  }

  return count;
}

unsigned int hitcon::uint_to_chr_hex(char *str, unsigned size, int n) {
  // Function to convert integer to hexadecimal
  if (n == 0) {
    str[0] = '0';
    str[1] = '\0';
    return 1;
  }

  int count = 0;
  while (n != 0 && count < size - 1) {
    int digit = n % 16;
    if (digit < 10) {
      str[count++] = '0' + digit;
    } else {
      str[count++] = 'A' + digit - 10;
    }
    n /= 16;
  }
  str[count] = '\0';

  // reverse the string
  char *p1 = str;
  char *p2 = str + strlen(p1) - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1++ = *p2;
    *p2-- = tmp;
  }

  return count;
}
