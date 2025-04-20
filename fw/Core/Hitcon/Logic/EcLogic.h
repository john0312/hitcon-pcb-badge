#ifndef SERVICE_EC_LOGIC_H_
#define SERVICE_EC_LOGIC_H_
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <stdint.h>

namespace hitcon {

namespace ecc {

namespace internal {

class ModNum {
  friend ModNum operator+(const uint64_t a, const ModNum &b);
  friend ModNum operator*(const uint64_t a, const ModNum &b);

 public:
  ModNum(uint64_t val, uint64_t mod);

  ModNum operator=(const uint64_t other);
  ModNum operator=(const ModNum &other);
  ModNum operator-() const;
  ModNum operator+(const ModNum &other) const;
  ModNum operator-(const ModNum &other) const;
  ModNum operator*(const ModNum &other) const;
  ModNum operator/(const ModNum &other) const;
  bool operator==(const ModNum &other) const;
  bool operator==(const uint64_t other) const;

  uint64_t val;
  uint64_t mod;
};

struct EllipticCurve {
  EllipticCurve(const uint64_t A, const uint64_t B);
  const uint64_t A, B;
};

class EcPoint {
 public:
  EcPoint();
  EcPoint(const ModNum &x, const ModNum &y);
  EcPoint operator=(const EcPoint &other);
  EcPoint operator-() const;
  EcPoint operator+(const EcPoint &other) const;
  EcPoint operator*(uint64_t times) const;
  bool operator==(const EcPoint &other) const;
  /**
   * Getter for the x-coordinate.
   */
  uint64_t xval() const;

 private:
  ModNum x, y;
  bool isInf;
  /**
   * Double the point. Returns (x + x).
   */
  EcPoint twice() const;
  /**
   * Find the intersection point given the slope.
   *
   * @param other the other point
   * @param l     the slope between `this` and `other`
   */
  EcPoint intersect(const EcPoint &other, const ModNum &l) const;
};

}  // namespace internal

struct Signature {
  internal::EcPoint pub;
  uint64_t r, s;
};

class EcLogic {
 public:
  EcLogic();

  void Init();

  /**
   * Start the signing process and mark this API as busy.
   * @param message: the message to sign. The contents should be intact until
   *                 sign finishes.
   * @param len: length of message, has to be a multiple of 8
   * @param callback: callback function to call when the sign is complete.
   *                  the second argument to callback is a pointer to
   *                  tmpSignature.
   * @param callbackArg1: the first argument to the callback. Normally a pointer
   * to "this" if the callback is a method, and nullptr if the callback is a
   * function.
   * @return whether the job is successfully queued.
   */
  bool StartSign(uint8_t const *message, uint32_t len, callback_t callback,
                 void *callbackArg1);

  /**
   * Start the verification process and mark this API as busy.
   * @param message: the message to sign. The contents should be intact until
   *                 verify finishes.
   * @param len: length of message, has to be a multiple of 8
   * @param signature: Signature object
   * @param callback: callback function to call when verification is complete.
   *                  the second argument to callback is whether the message is
   *                  valid.
   * @param callbackArg1: the first argument to the callback. Normally a pointer
   * to "this" if the callback is a method, and nullptr if the callback is a
   * function.
   * @return whether the job is successfully queued.
   */
  bool StartVerify(uint8_t const *message, uint32_t len,
                   const Signature &signature, callback_t callback,
                   void *callbackArg1);

 private:
  /**
   * Indicates whether a sign / verify operation is running.
   * If a task is active, this flag is set to true, and further requests to sign
   * / verify will fail. If neither sign nor verify is running, this flag should
   * be set to false.
   */
  bool busy;
  /**
   * Temporary storage of signature.
   * For the signing process, the data only lives since the signature completes
   * (at the end of doSign) till the callback returns. For the verification
   * process, the data lives since StartVerify is called till the callback
   * returns.
   */
  ecc::Signature tmpSignature;
  /**
   * Temporary storage of the message.
   * Lives since the public method (StartSign, StartVerify) is called till the
   * callback returns.
   */
  uint8_t const *savedMessage;
  uint32_t savedMessageLen;

  /**
   * Actual function to perform the sign.
   * Marks this API as not busy once the function returns.
   */
  void doSign(void *unused);

  /**
   * Actual function to perform the verification.
   * Marks this API as not busy once the function returns.
   */
  void doVerify(void *unused);

  callback_t callback;
  void *callback_arg1;

  service::sched::Task signTask;
  service::sched::Task verifyTask;
};

extern EcLogic g_ec_logic;

}  // namespace ecc

}  // namespace hitcon

#endif  // SERVICE_EC_LOGIC_H_