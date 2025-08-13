#include <Logic/EcLogic.h>
#include <Logic/RandomPool.h>
#include <Service/HashService.h>
#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>

#include <cstring>

using namespace hitcon::service::sched;
using namespace hitcon::ecc::internal;
using namespace hitcon::ecc;
using namespace hitcon::hash;

namespace hitcon {

namespace ecc {

EcLogic g_ec_logic;

namespace internal {

static constexpr uint64_t UINT64_MSB = 1ULL << 63;

// Hardcoded curve parameters
static const EllipticCurve g_curve(0x5e924cd447a56b, 0x892f0a953f589b);
static const EcPoint g_generator({0x9a77dc33b36acc, 0xbcffb098340493},
                                 {0x279be90a95dbdd, 0xbcffb098340493});
static const uint64_t g_curveOrder = 0xbcffb09c43733d;
// TODO: use GetPerBoardSecret to set the private key
static const EcPoint g_serverPubKey({0x05cb6b63de507e, 0xbcffb098340493},
                                    {0x4df751a1388b25, 0xbcffb098340493});

constexpr inline uint64_t modneg(const uint64_t x, const uint64_t m) {
  return m - (x % m);
}

constexpr inline uint64_t modadd(const uint64_t a, const uint64_t b,
                                 const uint64_t m) {
  if (a > UINT64_MAX - b)
    return modneg((modneg(a, m) + modneg(b, m)) % m, m);
  else
    return (a + b) % m;
}

constexpr inline uint64_t modsub(const uint64_t a, const uint64_t b,
                                 const uint64_t m) {
  if (a >= b)
    return a - b;
  else
    return a + m - b;
}

ModNum::ModNum(uint64_t val, uint64_t mod) : val(val), mod(mod) {}

ModNum ModNum::operator=(const ModNum &other) {
  val = other.val;
  mod = other.mod;
  return *this;
}

ModNum ModNum::operator=(const uint64_t other) {
  val = other % mod;
  return *this;
}

ModNum ModNum::operator-() const { return ModNum(modneg(val, mod), mod); }

ModNum ModNum::operator+(const ModNum &other) const {
  return ModNum(modadd(val, other.val, mod), mod);
}

ModNum operator+(const uint64_t a, const ModNum &b) {
  return ModNum(a, b.mod) + b;
}

ModNum ModNum::operator-(const ModNum &other) const {
  uint64_t a = val, b = other.val, m = mod;
  return ModNum(modsub(a, b, m), m);
}

bool ModNum::operator==(const ModNum &other) const {
  return val == other.val && mod == other.mod;
}

bool ModNum::operator==(const uint64_t other) const { return val == other; }

ModMulService g_mod_mul_service;

ModMulService::ModMulService()
    : routineTask(804, (callback_t)&ModMulService::routineFunc, this),
      finalizeTask(804, (callback_t)&ModMulService::finalize, this) {}

void ModMulService::start(uint64_t a, uint64_t b, uint64_t m,
                          callback_t callback, void *callbackArg1) {
  this->callback = callback;
  this->callbackArg1 = callbackArg1;
  context.a = a;
  context.b = b;
  context.m = m;
  context.res = 0;
  context.i = 0;
  scheduler.Queue(&routineTask, nullptr);
}

void ModMulService::routineFunc() {
  do {
    context.res = modadd(context.res, context.res, context.m);
    if (context.b & UINT64_MSB)
      context.res = modadd(context.res, context.a, context.m);
    context.b <<= 1;
    ++context.i;
  } while (context.i & 0b11111);
  if (context.i == 64)
    scheduler.Queue(&finalizeTask, nullptr);
  else
    scheduler.Queue(&routineTask, nullptr);
}

void ModMulService::finalize() { callback(callbackArg1, &context.res); }

ModDivService g_mod_div_service;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
ModDivService::ModDivService()
    : routineTask(803, (callback_t)&ModDivService::routineFunc, this),
      finalizeTask(803, (callback_t)&ModDivService::finalize, this) {}
#pragma GCC diagnostic pop

void ModDivService::start(uint64_t a, uint64_t b, uint64_t m,
                          callback_t callback, void *callbackArg1) {
  this->callback = callback;
  this->callbackArg1 = callbackArg1;
  context.a = a;
  context.m = m;
  context.ppr = b;
  context.pr = m;
  context.r = 0;
  context.ppx = 1;
  context.px = 0;
  context.q = 0;
  scheduler.Queue(&routineTask, this);
}

void ModDivService::routineFunc() {
  if (context.pr == 1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    g_mod_mul_service.start(context.a, context.px, context.m,
                            (callback_t)&ModDivService::preFinalize, this);
#pragma GCC diagnostic pop
    return;
  }
  context.q = context.ppr / context.pr;
  context.r = context.ppr % context.pr;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_mul_service.start(context.q, context.px, context.m,
                          (callback_t)&ModDivService::onModMulDone, this);
#pragma GCC diagnostic pop
}

void ModDivService::onModMulDone(uint64_t *x) {
  uint64_t _x = modsub(context.ppx, *x, context.m);
  context.ppr = context.pr;
  context.ppr = context.pr;
  context.pr = context.r;
  context.ppx = context.px;
  context.px = _x;
  scheduler.Queue(&routineTask, this);
}

void ModDivService::preFinalize(uint64_t *res) {
  context.res = *res;
  scheduler.Queue(&finalizeTask, nullptr);
}

void ModDivService::finalize() {
  ModNum res(context.res, context.m);
  callback(callbackArg1, &res);
}

EllipticCurve::EllipticCurve(const uint64_t A, const uint64_t B) : A(A), B(B) {}

EcPoint::EcPoint() : x{0, 0}, y{0, 0}, isInf(true) {}

EcPoint::EcPoint(const ModNum &x, const ModNum &y) : x(x), y(y), isInf(false) {}

EcPoint EcPoint::operator=(const EcPoint &other) {
  isInf = other.isInf;
  x = other.x;
  y = other.y;
  return *this;
}

EcPoint EcPoint::operator-() const {
  if (isInf) return EcPoint(*this);
  return EcPoint(x, -y);
}

bool EcPoint::operator==(const EcPoint &other) const {
  if (isInf) return other.isInf;
  return x == other.x && y == other.y;
}

uint64_t EcPoint::xval() const { return x.val; }

bool EcPoint::identity() const { return isInf; }

bool EcPoint::getCompactForm(uint8_t *buffer, size_t len) const {
  // Check if the point is the identity element (point at infinity)
  if (isInf) {
    return false;
  }

  // The compact form includes least significant 7 byte of the x-coordinate
  // and a sign byte.
  // We assume the x-coordinate is lesser than 2^56, so we need 64-bit.
  if (len < sizeof(uint64_t)) {
    return false;  // Buffer is too small
  }

  // Copy the x-coordinate value (uint64_t) into the buffer.
  // Note: We assume the environment is little-endian.
  memcpy(buffer, &(x.val), sizeof(uint64_t));

  uint8_t sign_bit = y.val & 1;

  // The sign bit is stored in the MSB of the last byte
  // of the output buffer. Since we copied sizeof(uint64_t) bytes, the last
  // byte containing x-coordinate data is at index sizeof(uint64_t) - 1.
  size_t last_byte_idx = sizeof(uint64_t) - 1;

  // Clear the MSB of this byte
  my_assert(buffer[last_byte_idx] == 0);

  if (sign_bit) {
    buffer[last_byte_idx] = 0x01;
  }
  return true;
}

PointAddContext::PointAddContext() : l(0, 1) {}

PointAddService g_point_add_service;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
PointAddService::PointAddService()
    : routineTask(802, (callback_t)&PointAddService::routineFunc, this),
      finalizeTask(802, (callback_t)&PointAddService::finalize, this),
      genXTask(802, (callback_t)&PointAddService::genXStep1, this),
      genYTask(802, (callback_t)&PointAddService::genYStep1, this) {}
#pragma GCC diagnostic pop

void PointAddService::start(const EcPoint &a, const EcPoint &b,
                            callback_t callback, void *callbackArg1) {
  context.a = a;
  context.b = b;
  this->callback = callback;
  this->callbackArg1 = callbackArg1;
  scheduler.Queue(&routineTask, this);
}

void PointAddService::routineFunc() {
  if (context.a.identity()) {
    context.res = context.b;
    scheduler.Queue(&finalizeTask, this);
  } else if (context.b.identity()) {
    context.res = context.a;
    scheduler.Queue(&finalizeTask, this);
  } else if (context.a == -context.b) {
    context.res = EcPoint();
    scheduler.Queue(&finalizeTask, this);
  } else if (context.a == context.b) {
    // double the point
    // Original formula is 3 * x^2 + A, we calculate x^2 here by doing x * x
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    g_mod_mul_service.start(context.a.x.val, context.a.x.val, context.a.x.mod,
                            (callback_t)&PointAddService::onLtopDone, this);
#pragma GCC diagnostic pop
  } else {
    // intersect directly
    ModNum l_top = context.b.y - context.a.y;
    ModNum l_bot = context.b.x - context.a.x;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    g_mod_div_service.start(l_top.val, l_bot.val, l_top.mod,
                            (callback_t)&PointAddService::onDivDone, this);
#pragma GCC diagnostic pop
  }
}

void PointAddService::onLtopDone(uint64_t *l_top) {
  ModNum _l_top(*l_top, context.a.x.mod);
  // Original formula is 3 * x^2 + A, we perform addition 3 times to avoid
  // expensive operations.
  _l_top = _l_top + _l_top + _l_top + ModNum(g_curve.A, _l_top.mod);
  // same here for 2 * y
  ModNum l_bot = context.a.y + context.a.y;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_div_service.start(_l_top.val, l_bot.val, _l_top.mod,
                          (callback_t)&PointAddService::onDivDone, this);
#pragma GCC diagnostic pop
}

void PointAddService::onDivDone(ModNum *l) {
  context.l = *l;
  scheduler.Queue(&genXTask, this);
}

void PointAddService::genXStep1() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_mul_service.start(context.l.val, context.l.val, context.l.mod,
                          (callback_t)&PointAddService::genXStep2, this);
#pragma GCC diagnostic pop
}

void PointAddService::genXStep2(uint64_t *lPow2) {
  ModNum _lPow2(*lPow2, context.l.mod);
  context.res.x = _lPow2 - context.a.x - context.b.x;
  scheduler.Queue(&genYTask, this);
}

void PointAddService::genYStep1() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_mul_service.start(context.l.val, (context.a.x - context.res.x).val,
                          context.l.mod,
                          (callback_t)&PointAddService::genYStep2, this);
#pragma GCC diagnostic pop
}

void PointAddService::genYStep2(uint64_t *lDx) {
  ModNum _lDx(*lDx, context.l.mod);
  context.res.y = _lDx - context.a.y;
  scheduler.Queue(&finalizeTask, this);
}

void PointAddService::finalize() { callback(callbackArg1, &context.res); }

PointMultContext::PointMultContext() : p(g_generator), res(g_generator) {}

PointMultService g_point_mult_service;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
PointMultService::PointMultService()
    : routineTask(801, (task_callback_t)&PointMultService::routineFunc,
                  (void *)this) {}
#pragma GCC diagnostic pop

void PointMultService::routineFunc() {
  if (context.i == 128) {
    callback(callbackArg1, &context.res);
  } else {
    if (context.i & 1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
      if (context.times & UINT64_MSB)
        g_point_add_service.start(context.p, context.res,
                                  (callback_t)&PointMultService::onAddDone,
                                  this);
      else
        scheduler.Queue(&routineTask, this);
#pragma GCC diagnostic pop
      context.times <<= 1;
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
      g_point_add_service.start(context.res, context.res,
                                (callback_t)&PointMultService::onAddDone, this);
#pragma GCC diagnostic pop
    }
    ++context.i;
  }
}

void PointMultService::onAddDone(EcPoint *res) {
  context.res = *res;
  scheduler.Queue(&routineTask, this);
}

void PointMultService::start(const EcPoint &p, uint64_t times,
                             callback_t callback, void *callbackArg1) {
  context.p = p;
  context.times = times;
  context.i = 0;
  context.res = EcPoint();
  this->callback = callback;
  this->callbackArg1 = callbackArg1;
  scheduler.Queue(&routineTask, this);
}

}  // namespace internal

void Signature::toBuffer(uint8_t *buffer) const {
  memcpy(buffer, &r, ECC_SIGNATURE_SIZE / 2);
  memcpy(buffer + ECC_SIGNATURE_SIZE / 2, &s, ECC_SIGNATURE_SIZE / 2);
}

void Signature::fromBuffer(const uint8_t *buffer) {
  r = 0;
  s = 0;
  memcpy(&r, buffer, ECC_SIGNATURE_SIZE / 2);
  memcpy(&s, buffer + ECC_SIGNATURE_SIZE / 2, ECC_SIGNATURE_SIZE / 2);
}

EcContext::EcContext()
    : r(0, g_curveOrder), s(0, g_curveOrder), u1(0, g_curveOrder),
      u2(0, g_curveOrder) {}

bool EcLogic::StartSign(uint8_t const *message, uint32_t len,
                        callback_t callback, void *callbackArg1) {
  if (busy) return false;
  if (!publicKeyReady) return false;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  if (!g_hash_service.StartHash(message, len,
                                (callback_t)&EcLogic::onSignHashFinish, this))
    return false;
#pragma GCC diagnostic pop
  busy = true;
  this->callback = callback;
  this->callback_arg1 = callbackArg1;
  return true;
}

bool EcLogic::StartVerify(uint8_t const *message, uint32_t len,
                          uint8_t *signature, callback_t callback,
                          void *callbackArg1) {
  if (busy) return false;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  if (!g_hash_service.StartHash(message, len,
                                (callback_t)&EcLogic::onVerifyHashFinish, this))
    return false;
#pragma GCC diagnostic pop
  busy = true;
  // Init the context
  tmpSignature.fromBuffer(signature);
  context.r = tmpSignature.r;
  context.s = tmpSignature.s;
  this->callback = callback;
  this->callback_arg1 = callbackArg1;
  return true;
}

void EcLogic::onSignHashFinish(HashResult *hashResult) {
  context.z =
      reinterpret_cast<uint64_t *>(hashResult->digest)[0] % g_curveOrder;
  scheduler.Queue(&genRandTask, this);
}

void EcLogic::onVerifyHashFinish(HashResult *HashResult) {
  context.z =
      reinterpret_cast<uint64_t *>(HashResult->digest)[0] % g_curveOrder;
  // u1 = z / s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_div_service.start(context.z, context.s.val, g_curveOrder,
                          (callback_t)&EcLogic::onU1Generated, this);
#pragma GCC diagnostic pop
}

void EcLogic::genRand() {
  context.k = ((uint64_t)(g_fast_random_pool.GetRandom())) << 32 |
              g_fast_random_pool.GetRandom();
  // r = k * G
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_point_mult_service.start(g_generator, context.k,
                             (callback_t)&EcLogic::onRGenerated, this);
#pragma GCC diagnostic pop
}

void EcLogic::onRGenerated(EcPoint *p) {
  context.r = p->xval();
  if (context.r == 0)
    scheduler.Queue(&genRandTask, this);
  else {
    // Start generating S
    // We start by calculating r * d
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    g_mod_mul_service.start(privateKey, context.r.val, context.r.mod,
                            (callback_t)&EcLogic::genS, this);
#pragma GCC diagnostic pop
  }
}

void EcLogic::genS(uint64_t *pkR) {
  ModNum a = context.z + ModNum(*pkR, context.r.mod);
  // s = (z + r * d) / k
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_div_service.start(a.val, context.k, a.mod,
                          (callback_t)&EcLogic::onSGenerated, this);
#pragma GCC diagnostic pop
}

void EcLogic::onSGenerated(ModNum *s) {
  context.s = *s;
  if (context.s == 0)
    scheduler.Queue(&genRandTask, this);
  else
    scheduler.Queue(&finalizeTask, this);
}

void EcLogic::finalizeSign() {
  tmpSignature.r = context.r.val;
  tmpSignature.s = context.s.val;
  callback(callback_arg1, &tmpSignature);
  busy = false;
}

void EcLogic::onU1Generated(ModNum *u1) {
  context.u1 = *u1;
  // u2 = r / s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_mod_div_service.start(context.r.val, context.s.val, g_curveOrder,
                          (callback_t)&EcLogic::onU2Generated, this);
#pragma GCC diagnostic pop
}

void EcLogic::onU2Generated(ModNum *u2) {
  context.u2 = *u2;
  // normally, the next step is P = u1 * G + u2 * pub
  // but we do it separately:
  // m = u1 * G
  // n = u2 * pub
  // P = m + n
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_point_mult_service.start(g_generator, context.u1.val,
                             (callback_t)&EcLogic::onMGenerated, this);
#pragma GCC diagnostic pop
}

void EcLogic::onMGenerated(EcPoint *m) {
  context.m = *m;
  // n = u2 * pub
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_point_mult_service.start(g_serverPubKey, context.u2.val,
                             (callback_t)&EcLogic::onNGenerated, this);
#pragma GCC diagnostic pop
}

void EcLogic::onNGenerated(EcPoint *n) {
  // P = m + n
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_point_add_service.start(context.m, *n, (callback_t)&EcLogic::finalizeVerify,
                            this);
#pragma GCC diagnostic pop
}

void EcLogic::finalizeVerify(EcPoint *P) {
  // P == identity -> signature is invalid
  // otherwise, check if r == P.x
  callback(callback_arg1,
           (void *)(!P->identity() && context.r.val == P->xval()));
  busy = false;
}

void EcLogic::onPubkeyDone(EcPoint *p) {
  // Ensure the derived point is not the point at infinity
  // A private key of 0 or a multiple of the curve order would result in
  // infinity
  if (!p->identity()) {
    bool ret = p->getCompactForm(publicKey, ECC_PUBKEY_SIZE);
    if (ret) publicKeyReady = true;
  } else {
    publicKeyReady = false;
  }
}

const uint8_t *EcLogic::GetPublicKey() {
  if (publicKeyReady) {
    return &publicKey[0];
  }
  return nullptr;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
EcLogic::EcLogic()
    : genRandTask(800, (callback_t)&EcLogic::genRand, this),
      finalizeTask(800, (callback_t)&EcLogic::finalizeSign, this) {}
#pragma GCC diagnostic pop

void EcLogic::SetPrivateKey(uint64_t privkey) {
  privateKey = privkey;
  privateKey = privateKey % g_curveOrder;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_point_mult_service.start(g_generator, privateKey,
                             (callback_t)&EcLogic::onPubkeyDone, this);
#pragma GCC diagnostic pop
}

}  // namespace ecc

}  // namespace hitcon
