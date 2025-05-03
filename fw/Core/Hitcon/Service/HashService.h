#ifndef HASH_SERVICE_H
#define HASH_SERVICE_H

#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

class HashService {
 public:
  bool StartHash(uint8_t const *message, size_t len, callback_t callback,
                 void *callback_arg1);
};

extern HashService g_hash_service;

}  // namespace hitcon

#endif  // HASH_SERVICE_H