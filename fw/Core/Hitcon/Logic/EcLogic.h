#ifndef SERVICE_EC_LOGIC_H_
#define SERVICE_EC_LOGIC_H_
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <stdint.h>

namespace hitcon {

// hardcoded: server public key, curve params (a, b, n, G, p)

class EcPoint {
  // TODO: define this class
};

struct Signature {
  const EcPoint pub;
  const uint64_t r, s;
};

class EcLogic {
 public:
  EcLogic();

  void Init();

  /**
   * Start the signing process.
   * @param message: the message to sign
   * @param len: length of message, has to be a multiple of 8
   * @param callback: callback function to call when the sign is complete.
   *                  the second argument to callback is a pointer to
   *                  g_generated_signature.
   * @return whether the job is successfully queued.
   */
  bool StartSign(const uint8_t *message, uint32_t len, callback_t callback);

  /**
   * Start the verification process.
   * @param message: the message to sign
   * @param len: length of message, has to be a multiple of 8
   * @param signature: Signature object
   * @param callback: callback function to call when verification is complete.
   *                  the second argument to callback is whether the message is
   * valid.
   * @return whether the job is successfully queued.
   */
  bool StartVerify(const uint8_t *message, uint32_t len,
                   const Signature &signature, callback_t callback);

 private:
  /**
   * This object only lives until the callback method returns.
   */
  Signature g_generated_signature;
};
extern EcLogic g_ec_logic;

}  // namespace hitcon

#endif  // SERVICE_EC_LOGIC_H_