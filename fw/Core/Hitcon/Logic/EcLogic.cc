#include <Logic/EcLogic.h>
#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>

using namespace hitcon::service::sched;
using namespace hitcon::ecc::internal;
using namespace hitcon::ecc;

namespace hitcon {

EcLogic g_ec_logic;

namespace ecc {

namespace internal {

#define UINT64_MSB (1ULL << 63)

// Hardcoded curve parameters
static const EllipticCurve g_curve(0x5e924cd447a56b, 0x892f0a953f589b);
static const EcPoint g_generator({0x9a77dc33b36acc, 0xbcffb098340493},
                                 {0x279be90a95dbdd, 0xbcffb098340493});
static const uint64_t g_curveOrder = 0xbcffb09c43733d;
// TODO: use GetPerBoardSecret to set the private key
static const uint64_t g_privateKey = 87;

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

ModNum ModNum::operator/(const ModNum &other) const {
  uint64_t m = mod;
  uint64_t x = extgcd(other.val, m);
  return ModNum(modmul(val, x, m), m);
}

bool ModNum::operator==(const ModNum &other) const {
  return val == other.val && mod == other.mod;
}

bool ModNum::operator==(const uint64_t other) const { return val == other; }

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

EcPoint EcPoint::operator+(const EcPoint &other) const {
  if (isInf) return EcPoint(other);
  if (other.isInf) return EcPoint(*this);
  if (*this == -other) return EcPoint();
  if (*this == other) return twice();
  ModNum l = (other.y - y) / (other.x - x);
  return intersect(other, l);
}

EcPoint EcPoint::operator*(uint64_t times) const {
  EcPoint res;
  for (int i = 0; i < 64; ++i) {
    res = res + res;
    if (times & UINT64_MSB) res = res + *this;
    times <<= 1;
  }
  return res;
}

bool EcPoint::operator==(const EcPoint &other) const {
  if (isInf) return other.isInf;
  return x == other.x && y == other.y;
}

uint64_t EcPoint::xval() const { return x.val; }

EcPoint EcPoint::twice() const {
  ModNum l = (3 * x * x + ModNum(g_curve.A, x.mod)) / (2 * y);
  return intersect(*this, l);
}

EcPoint EcPoint::intersect(const EcPoint &other, const ModNum &l) const {
  ModNum newx = l * l - x - other.x;
  ModNum newy = l * (x - newx) - y;
  return EcPoint(newx, newy);
}

}  // namespace internal

uint64_t computeHash(uint8_t const *message, uint32_t len) {
  // TODO: call the sha3 api
  return 0xcafebabedeadbeef;
}

uint64_t getRandValue() {
  // TODO: call the random api
  return 0xdeadbeefcafebabe;
}

bool EcLogic::StartSign(uint8_t const *message, uint32_t len,
                        callback_t callback, void *callbackArg1) {
  if (busy) return false;
  if (!scheduler.Queue(&signTask, nullptr)) return false;
  busy = true;
  this->callback = callback;
  this->callback_arg1 = callbackArg1;
  this->savedMessage = message;
  this->savedMessageLen = len;
  return true;
}

bool EcLogic::StartVerify(uint8_t const *message, uint32_t len,
                          const Signature &signature, callback_t callback,
                          void *callbackArg1) {
  if (busy) return false;
  if (!scheduler.Queue(&verifyTask, nullptr)) return false;
  busy = true;
  this->callback = callback;
  this->callback_arg1 = callbackArg1;
  this->savedMessage = message;
  this->savedMessageLen = len;
  return true;
}

void EcLogic::doSign(void *unused) {
  uint64_t z = computeHash(savedMessage, savedMessageLen);
  ModNum r(0, g_curveOrder), s(0, g_curveOrder);
  while (s == 0) {
    ModNum k(0, g_curveOrder);
    while (r == 0) {
      k = getRandValue();
      EcPoint P = g_generator * k.val;
      r = P.xval();
    }
    s = (z + g_privateKey * r) / k;
  }
  tmpSignature.pub = g_generator * g_privateKey;
  tmpSignature.r = r.val;
  tmpSignature.s = s.val;
  callback(callback_arg1, &tmpSignature);
  busy = false;
}

void EcLogic::doVerify(void *unused) {
  ModNum z(computeHash(savedMessage, savedMessageLen), g_curveOrder);
  ModNum u1 = z / ModNum(tmpSignature.s, g_curveOrder);
  ModNum u2 = ModNum(tmpSignature.r, g_curveOrder) /
              ModNum(tmpSignature.s, g_curveOrder);
  EcPoint P = g_generator * u1.val + tmpSignature.pub * u2.val;
  bool isValid = P.xval() == tmpSignature.r;
  callback(callback_arg1, (void *)isValid);
  busy = false;
}

EcLogic::EcLogic()
    : signTask(800, (task_callback_t)&EcLogic::doSign, (void *)&g_ec_logic),
      verifyTask(800, (task_callback_t)&EcLogic::doVerify,
                 (void *)&g_ec_logic) {}

void EcLogic::Init() {
  // TODO: initialize the private key here maybe?
}

}  // namespace ecc

}  // namespace hitcon