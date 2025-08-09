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
  bool operator==(const ModNum &other) const;
  bool operator==(const uint64_t other) const;

  uint64_t val;
  uint64_t mod;
};

/**
 * Context for performing res = (a / b) mod m.
 * Algorithm is taken from here:
 * https://zerobone.net/blog/math/extended-euklidean-algorithm/
 */
struct ModDivContext {
  uint64_t m;
  uint64_t ppr, pr;
  uint64_t ppx, px;
  uint64_t a;
  uint64_t res;
};

class ModDivService {
 public:
  void start(uint64_t a, uint64_t b, uint64_t m, callback_t callback,
             void *callbackArg1);
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
  friend class PointAddService;

 public:
  EcPoint();
  EcPoint(const ModNum &x, const ModNum &y);
  EcPoint operator=(const EcPoint &other);
  EcPoint operator-() const;
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
   * Convert to compact form (x and the last byte storing 0 or 1 deciding
   * if it's positive or negative).
   * Output in little endian, and this can only run on little endian.
   * len is the size of buffer. Return false if it doesn't fit.
   */
  bool getCompactForm(uint8_t *buffer, size_t len) const;

 private:
  bool isInf;
  ModNum x, y;
};

/**
 * Context for res = a + b.
 * This is done by calculating the slope l between a and b, then intersecting it
 * with the curve.
 */
struct PointAddContext {
  EcPoint a;
  EcPoint b;
  EcPoint res;
  // Storage for the slope.
  ModNum l;
  PointAddContext();
};

class PointAddService {
 public:
  void start(const EcPoint &a, const EcPoint &b, callback_t callback,
             void *callbackArg1);
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

/**
 * Context for res = p * times.
 * We do this similarly to modular exponentiation, where we iterate through 64
 * bits and do a point addition according to each bit.
 */
struct PointMultContext {
  EcPoint p;
  uint64_t times;
  EcPoint res;
  // The iterator.
  uint8_t i;
  PointMultContext();
};

class PointMultService {
 public:
  void start(const EcPoint &p, uint64_t times, callback_t callback,
             void *callbackArg1);
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
  // hash of the message
  uint64_t z;
  // signature
  ModNum r, s;
  /* --- Signing context --- */
  // a random value
  uint64_t k;
  /* --- Verifying context --- */
  ModNum u1, u2;
  // temporal storage for a point
  EcPoint m;
  EcContext();
};

}  // namespace internal

struct Signature {
  uint64_t r, s;

  /**
   * Dump the signature to a buffer. Buffer should be at least
   * ECC_SIGNATURE_SIZE. This function does not perform any size checks! Caller
   * is responsible for it.
   *
   * @param buffer: The buffer to write the signature to.
   */
  void toBuffer(uint8_t *buffer) const;

  /**
   * Load the signature from a buffer. Buffer should be at least
   * ECC_SIGNATURE_SIZE. This funciton does not perform any size checks!
   * Caller is responsible for it.
   *
   * @param buffer: The buffer containing the signature.
   */
  void fromBuffer(const uint8_t *buffer);
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
   * Retrieve the public key.
   *
   * @return A pointer to the public key, it'll hold ECC_PUBKEY_SIZE bytes.
   *
   * Note that the returned buffer may no longer be valid after the current
   * task ends.
   */
  const uint8_t *GetPublicKey();

  /**
   * Start the signing process and mark this API as busy.
   *
   * @param message:      the message to sign. The contents should be intact
   *                      until sign finishes.
   * @param len:          length of message.
   * @param callback:     callback function to call when the sign is complete.
   *                      The second argument to callback is a pointer to
   *                      tmpSignature.
   * @param callbackArg1: The first argument to the callback. Normally a
   *                      pointer to "this" if the callback is a method, and
   *                      nullptr if the callback is a function.
   * @return              whether the job is successfully queued.
   */
  bool StartSign(uint8_t const *message, uint32_t len, callback_t callback,
                 void *callbackArg1);

  /**
   * Start the verification process and mark this API as busy.
   * This will only verify the signature against the server public key.
   *
   * @param message:      the message to verify. The contents should be intact
   *                      until verify finishes.
   * @param len:          length of message.
   * @param signature:    The signature to verify in raw form. Must be
   *                      ECC_SIGNATURE_SIZE bytes in size. This function does
   *                      not perform size checks! Caller is responsible.
   * @param callback:     callback function to call when verification is
   *                      complete.
   *                      The second argument to callback is a boolean value
   *                      indicating whether the signature is valid or not.
   * @param callbackArg1: The first argument to the callback. Normally a
   *                      pointer to "this" if the callback is a method, and
   *                      nullptr if the callback is a function.
   * @return              whether the job is successfully queued.
   */
  bool StartVerify(uint8_t const *message, uint32_t len, uint8_t *signature,
                   callback_t callback, void *callbackArg1);

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
  void onSignHashFinish(hitcon::hash::HashResult *hashResult);
  void onVerifyHashFinish(hitcon::hash::HashResult *hashResult);
  void onRGenerated(internal::EcPoint *p);
  void onSGenerated(internal::ModNum *s);
  void finalizeSign();
  void onU1Generated(internal::ModNum *u1);
  void onU2Generated(internal::ModNum *u2);
  void onMGenerated(internal::EcPoint *m);
  void onNGenerated(internal::EcPoint *n);
  void finalizeVerify(internal::EcPoint *P);

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
