#ifndef HASH_SERVICE_H
#define HASH_SERVICE_H

#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

class HashService {
 public:
  void Init();

  // Call StartHash() to hash the message of size len.
  // Return false if HashService is busy and cannot take this request. In that
  // case the caller should retry later.
  // Return true if HashService has accepted this request, in that case the
  // callback will be called once the hashing is done. The argument to the
  // callback will be a uint8_t pointer to the hash result, it'll only be valid
  // during the callback.
  bool StartHash(uint8_t const *message, size_t len, callback_t callback,
                 void *callback_arg1);
};

extern HashService g_hash_service;

}  // namespace hitcon

#endif  // HASH_SERVICE_H