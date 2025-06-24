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

#define UINT64_MSB (1ULL << 63)

// Hardcoded curve parameters
static const EllipticCurve g_curve(0x5e924cd447a56b, 0x892f0a953f589b);
static const EcPoint g_generator({0x9a77dc33b36acc, 0xbcffb098340493},
                                 {0x279be90a95dbdd, 0xbcffb098340493});
static const uint64_t g_curveOrder = 0xbcffb09c43733d;
// TODO: use GetPerBoardSecret to set the private key

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

inline uint64_t modmul(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t res = 0;
  for (int i = 0; i < 64; ++i) {
    res = modadd(res, res, m);
    if (b & UINT64_MSB) res = modadd(res, a, m);
    b <<= 1;
  }
  return res;
}

uint64_t extgcd(uint64_t ppr, uint64_t pr) {
  // return only coefficient of a
  // only works under mod pr
  // ignores division by zeroes
  uint64_t m = pr;
  uint64_t ppx = 1;
  uint64_t px = 0;
  while (pr != 1) {
    uint64_t q = ppr / pr;
    uint64_t r = ppr % pr;
    uint64_t x = modsub(ppx, modmul(q, px, m), m);
    ppr = pr;
    pr = r;
    ppx = px;
    px = x;
  }
  return px;
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

ModNum ModNum::operator*(const ModNum &other) const {
  uint64_t a = val, b = other.val, m = mod;
  return ModNum(modmul(a, b, m), m);
}

ModNum operator*(const uint64_t a, const ModNum &b) {
  return ModNum(a, b.mod) * b;
}

bool ModNum::operator==(const ModNum &other) const {
  return val == other.val && mod == other.mod;
}

bool ModNum::operator==(const uint64_t other) const { return val == other; }

ModDivService g_mod_div_service;

ModDivService::ModDivService()
    : routineTask(803, (callback_t)&ModDivService::routineFunc, this),
      finalizeTask(803, (callback_t)&ModDivService::finalize, this) {}

void ModDivService::start(uint64_t a, uint64_t b, uint64_t m,
                          callback_t callback, void *callbackArg1) {
  this->callback = callback;
  this->callbackArg1 = callbackArg1;
  context.a = a;
  context.m = m;
  context.ppr = b;
  context.pr = m;
  context.ppx = 1;
  context.px = 0;
  context.res = 0;
  scheduler.Queue(&routineTask, this);
}

void ModDivService::routineFunc() {
  if (context.pr == 1) {
    context.res = modmul(context.a, context.px, context.m);
    scheduler.Queue(&finalizeTask, this);
    return;
  }
  uint64_t q = context.ppr / context.pr;
  uint64_t r = context.ppr % context.pr;
  uint64_t x = modsub(context.ppx, modmul(q, context.px, context.m), context.m);
  context.ppr = context.pr;
  context.pr = r;
  context.ppx = context.px;
  context.px = x;
  scheduler.Queue(&routineTask, this);
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

PointAddService::PointAddService()
    : routineTask(802, (callback_t)&PointAddService::routineFunc, this),
      finalizeTask(802, (callback_t)&PointAddService::finalize, this),
      genXTask(802, (callback_t)&PointAddService::genX, this),
      genYTask(802, (callback_t)&PointAddService::genY, this) {}

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
    // Original formula is 3 * x^2 + A, but we do the addition 3 times instead
    // to avoid the expensive multiplication.
    ModNum l_top = context.a.x * context.a.x;
    l_top = l_top + l_top + l_top + ModNum(g_curve.A, l_top.mod);
    // Same applies here, original formula is 2 * y
    ModNum l_bot = context.a.y + context.a.y;
    g_mod_div_service.start(l_top.val, l_bot.val, l_top.mod,
                            (callback_t)&PointAddService::onDivDone, this);
  } else {
    // intersect directly
    ModNum l_top = context.b.y - context.a.y;
    ModNum l_bot = context.b.x - context.a.x;
    g_mod_div_service.start(l_top.val, l_bot.val, l_top.mod,
                            (callback_t)&PointAddService::onDivDone, this);
  }
}

void PointAddService::onDivDone(ModNum *l) {
  context.l = *l;
  scheduler.Queue(&genXTask, this);
}

void PointAddService::genX() {
  context.res.x = context.l * context.l - context.a.x - context.b.x;
  scheduler.Queue(&genYTask, this);
}

void PointAddService::genY() {
  context.res.y = context.l * (context.a.x - context.res.x) - context.a.y;
  scheduler.Queue(&finalizeTask, this);
}

void PointAddService::finalize() { callback(callbackArg1, &context.res); }

PointMultContext::PointMultContext() : p(g_generator), res(g_generator) {}

PointMultService g_point_mult_service;

PointMultService::PointMultService()
    : routineTask(801, (task_callback_t)&PointMultService::routineFunc,
                  (void *)this) {}

void PointMultService::routineFunc() {
  if (context.i == 128) {
    callback(callbackArg1, &context.res);
  } else {
    if (context.i & 1) {
      if (context.times & UINT64_MSB)
        g_point_add_service.start(context.p, context.res,
                                  (callback_t)&PointMultService::onAddDone,
                                  this);
      else
        scheduler.Queue(&routineTask, this);
      context.times <<= 1;
    } else {
      g_point_add_service.start(context.res, context.res,
                                (callback_t)&PointMultService::onAddDone, this);
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

EcContext::EcContext() : r(0, g_curveOrder), s(0, g_curveOrder) {}

bool EcLogic::StartSign(uint8_t const *message, uint32_t len,
                        callback_t callback, void *callbackArg1) {
  if (busy) return false;
  if (!publicKeyReady) return false;
  if (!g_hash_service.StartHash(message, len,
                                (callback_t)&EcLogic::onHashFinish, this))
    return false;
  busy = true;
  this->callback = callback;
  this->callback_arg1 = callbackArg1;
  return true;
}

void EcLogic::onHashFinish(HashResult *hashResult) {
  context.z = reinterpret_cast<uint64_t *>(hashResult->digest)[0];
  scheduler.Queue(&genRandTask, this);
}

void EcLogic::genRand() {
  if (g_secure_random_pool.GetRandom(&context.k))
    g_point_mult_service.start(g_generator, context.k,
                               (callback_t)&EcLogic::onRGenerated, this);
  else
    scheduler.Queue(&genRandTask, this);
}

void EcLogic::onRGenerated(EcPoint *p) {
  context.r = p->xval();
  if (context.r == 0)
    scheduler.Queue(&genRandTask, this);
  else {
    ModNum a = (context.z + privateKey * context.r);
    g_mod_div_service.start(a.val, context.k, a.mod,
                            (callback_t)&EcLogic::onSGenerated, this);
  }
}

void EcLogic::onSGenerated(ModNum *s) {
  context.s = *s;
  if (context.s == 0)
    scheduler.Queue(&genRandTask, this);
  else
    scheduler.Queue(&finalizeTask, this);
}

void EcLogic::finalize() {
  tmpSignature.r = context.r.val;
  tmpSignature.s = context.s.val;
  callback(callback_arg1, &tmpSignature);
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

bool EcLogic::GetPublicKey(uint8_t *buffer) {
  if (publicKeyReady) {
    memcpy(buffer, publicKey, hitcon::ECC_PUBKEY_SIZE);
    return true;
  }
  return false;
}
EcLogic::EcLogic()
    : privateKey(0), publicKeyReady(0), busy(false),
      genRandTask(800, (callback_t)&EcLogic::genRand, this),
      finalizeTask(800, (callback_t)&EcLogic::finalize, this) {}

void EcLogic::SetPrivateKey(uint64_t privkey) {
  privateKey = privkey;
  privateKey = privateKey % g_curveOrder;
  g_point_mult_service.start(g_generator, privateKey,
                             (callback_t)&EcLogic::onPubkeyDone, this);
}

}  // namespace ecc

}  // namespace hitcon
