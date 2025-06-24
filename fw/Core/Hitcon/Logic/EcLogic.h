#ifndef SERVICE_EC_LOGIC_H_
#define SERVICE_EC_LOGIC_H_
#include <Service/EcParams.h>
#include <Service/HashService.h>
#include <Service/Sched/Task.h>
#include <Util/callback.h>
#include <stdint.h>
#include <stdlib.h>

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
  bool operator!=(const ModNum &other) const;

  uint64_t val;
  uint64_t mod;
};

struct ModDivContext {
  uint64_t m;
  uint64_t ppr, pr;
  uint64_t ppx, px;
  uint64_t a;
  uint64_t res;
};

class ModDivService {
 public:
  void start(uint64_t a, uint64_t b, uint64_t m, callback_t callback, void *callbackArg1);
  ModDivService();
 private:
  callback_t callback;
  void *callbackArg1;
  ModDivContext context;
  service::sched::Task routineTask;
  service::sched::Task finalizeTask;
  void routineFunc();
  void finalize();
};

extern ModDivService g_mod_div_service;

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
  /**
   * Whether the point is the identity element.
   */
  bool identity() const;
  /**
   * Whether this point is on a given curve.
   */
  bool onCurve(const EllipticCurve &curve) const;

  /**
   * Convert to compact form (x and the last byte storing 0 or 1 deciding
   * if it's positive or negative).
   * Output in little endian, and this can only run on little endian.
   * len is the size of buffer. Return false if it doesn't fit.
   */
  bool getCompactForm(uint8_t *buffer, size_t len) const;

  ModNum x, y;
 private:
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

struct PointAddContext {
  EcPoint a;
  EcPoint b;
  EcPoint res;
  ModNum l;
  PointAddContext();
};

class PointAddService {
 public:
  void start(const EcPoint &a, const EcPoint &b, callback_t callback, void *callbackArg1);
  PointAddService();
 private:
  callback_t callback;
  void *callbackArg1;
  PointAddContext context;
  service::sched::Task routineTask;
  service::sched::Task finalizeTask;
  service::sched::Task genXTask;
  service::sched::Task genYTask;
  void routineFunc();
  void genX();
  void genY();
  void onDivDone(ModNum *l);
  void finalize();
};

extern PointAddService g_point_add_service;

struct PointMultContext {
  EcPoint p;
  uint64_t times;
  uint8_t i;
  EcPoint res;
  PointMultContext();
};

class PointMultService {
 public:
  void start(const EcPoint &p, uint64_t times, callback_t callback, void *callbackArg1);
  PointMultService();
 private:
  callback_t callback;
  void *callbackArg1;
  PointMultContext context;
  service::sched::Task routineTask;
  void routineFunc();
  void onAddDone(EcPoint *res);
};

extern PointMultService g_point_mult_service;

struct EcContext {
  uint64_t z, k;
  ModNum r, s;
  EcContext();
};

}  // namespace internal

struct Signature {
  internal::EcPoint pub;
  uint64_t r, s;

  /**
   * Dump the signature to a buffer. Buffer should be at least
   * ECC_SIGNATURE_SIZE. This function does not perform any size checks! Caller
   * is responsible for it.
   */
  void toBuffer(uint8_t *buffer) const;
};

class EcLogic {
 public:
  EcLogic();

  /**
   * Set the private key used by this class.
   *
   * @param privkey: The private key.
   *
   * Note that this will automatically start the computation of public key.
   */
  void SetPrivateKey(uint64_t privkey);

  /**
   * Store the public key into the buffer.
   *
   * @param buffer: The buffer, must be ECC_PUBKEY_SIZE bytes in size.
   *
   * @return If the key is copied in. False when it's not ready.
   */
  bool GetPublicKey(uint8_t *buffer);

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

 private:
  /**
   * Indicates whether a sign / verify operation is running.
   * If a task is active, this flag is set to true, and further requests to sign
   * / verify will fail. If neither sign nor verify is running, this flag should
   * be set to false.
   */
  bool busy;
  /**
   * Temporary storage of the random value used for sig generation.
   */
  uint64_t tmpRandValue;
  /**
   * Temporary storage of signature.
   * For the signing process, the data only lives since the signature completes
   * (at the end of doSign) till the callback returns. For the verification
   * process, the data lives since StartVerify is called till the callback
   * returns.
   */
  ecc::Signature tmpSignature;

  /**
   * The private key.
   */
  uint64_t privateKey;

  /**
   * The public key.
   */
  uint8_t publicKey[ECC_PUBKEY_SIZE];
  uint8_t publicKeyReady;

  hitcon::ecc::internal::EcContext context;

  void genRand();
  void onHashFinish(hitcon::hash::HashResult *hashResult);
  void onRGenerated(internal::EcPoint *p);
  void onSGenerated(internal::ModNum *s);
  void finalize();

  void onPubkeyDone(internal::EcPoint *p);

  callback_t callback;
  void *callback_arg1;

  service::sched::Task genRandTask;
  service::sched::Task finalizeTask;
};

extern EcLogic g_ec_logic;

}  // namespace ecc

}  // namespace hitcon

#endif  // SERVICE_EC_LOGIC_H_
