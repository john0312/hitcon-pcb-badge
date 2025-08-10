#ifndef HASH_SERVICE_H
#define HASH_SERVICE_H

#include <Logic/keccak.h>
#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

namespace hash {

constexpr size_t SHA3_BIT_SIZE = 256;

namespace internal {

struct HashStatus {
  size_t progress;
  int round;
  enum state { kUpdateState, kFinalizeState, kDoneState } state;

  HashStatus();
  void Init();
  void NewState(enum state state);
};

struct ServiceContext {
  uint8_t const *message;
  size_t len;
  callback_t callback;
  void *callbackArg1;

  ServiceContext();
  void Init(uint8_t const *message, size_t len, callback_t callback,
            void *callbackArg1);
};

}  // namespace internal

struct HashResult {
  uint8_t *digest;
  size_t size;
};

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
  // It is guaranteed that the callback will only be called after StartHash()
  // returns.
  bool StartHash(uint8_t const *message, size_t len, callback_t callback,
                 void *callbackArg1);

  HashService();

 private:
  service::sched::PeriodicTask hashTask;

  internal::ServiceContext serviceContext;
  internal::HashStatus status;
  sha3_context sha3Context;
  HashResult result;

  void doHash(void *unused);

  void doHashUpdate();
  void doHashFinalize();
  void doHashDone();
};

extern HashService g_hash_service;

}  // namespace hash

}  // namespace hitcon

#endif  // HASH_SERVICE_H