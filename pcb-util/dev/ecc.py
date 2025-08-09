import hashlib
import random

class ModNum:
    def __init__(self, val: int, mod: int):
        assert isinstance(val, int)
        assert isinstance(mod, int)
        self.val = val % mod
        self.mod = mod

    def modArithAssert(self, other):
        assert isinstance(other, ModNum)
        assert self.mod == other.mod

    def modArithConvert(self, other):
        if isinstance(other, int):
            other = ModNum(other, self.mod)
        elif isinstance(other, ModNum):
            pass
        return other

    def __add__(self, other):
        other = self.modArithConvert(other)
        self.modArithAssert(other)
        return ModNum(self.val + other.val, self.mod)

    def __radd__(self, other):
        return self + other

    def __neg__(self):
        return ModNum(self.mod - self.val, self.mod)

    def __sub__(self, other):
        other = self.modArithConvert(other)
        self.modArithAssert(other)
        return ModNum(self.val + self.mod - other.val, self.mod)

    def __mul__(self, other):
        other = self.modArithConvert(other)
        self.modArithAssert(other)
        return ModNum(self.val * other.val, self.mod)

    def __rmul__(self, other):
        return self * other

    def __rfloordiv__(self, other):
        if isinstance(other, int):
            return ModNum(other, self.mod) // self
        else:
            raise TypeError(f'Unsupported // between {type(other)} and ModNum')

    def __floordiv__(self, other):
        other = self.modArithConvert(other)
        self.modArithAssert(other)
        x = gcdExtend(other.val, self.mod)
        # bx + my = 1
        # x = b'
        return self * ModNum(x, self.mod)

    def __repr__(self):
        return f'{self.val} (%{self.mod})'

    def __pow__(self, exp):
        assert isinstance(exp, int) and exp > 0

        if exp == 1:
            return ModNum(self.val, self.mod)
        sq = self ** (exp // 2)
        sq *= sq
        if exp % 2:
            return sq * self
        else:
            return sq

    def __gt__(self, other):
        if isinstance(other, int):
            return self.val > other
        elif isinstance(other, ModNum):
            return self.val > other.val
        else:
            raise TypeError(f'gt between ModNum and {type(other)} not supported')

    def __eq__(self, other):
        if isinstance(other, int):
            return self.val == other
        elif isinstance(other, ModNum):
            return self.val == other.val and self.mod == other.mod
        else:
            raise TypeError(f'eq between ModNum and {type(other)} not supported')

    def __ge__(self, other):
        return self == other or self > other

    def __lt__(self, other):
        if isinstance(other, int):
            return self.val < other
        elif isinstance(other, ModNum):
            return self.val < other.val
        else:
            raise TypeError(f'lt between ModNum and {type(other)} not supported')

    def __le__(self, other):
        return self == other or self < other

def gcdExtend(ppr: int, pr: int):
    m = pr
    ppx = 1
    px = 0
    while pr != 1:
        q = ppr // pr
        r = ppr % pr
        x = (ppx - q * px) % m

        ppr, pr = pr, r
        ppx, px = px, x
    return px

class ECurve:
    """
    Defines the montgomery curve: y^2 = x^3 + Ax + B
    """
    def __init__(self, A, B):
        assert isinstance(A, int)
        assert isinstance(B, int)
        self.A = A
        self.B = B

    def __eq__(self, other):
        assert isinstance(other, ECurve)
        return self.A == other.A and self.B == other.B

class EPoint:
    x: ModNum
    y: ModNum
    curve: ECurve

    def __init__(self, x: ModNum, y: ModNum, curve: ECurve):
        assert isinstance(x, ModNum)
        assert isinstance(y, ModNum)
        assert x.mod == y.mod
        assert isinstance(curve, ECurve)
        self.x = x
        self.y = y
        self.curve = curve

    def pointArithAssert(self, other):
        assert isinstance(other, EPoint)
        assert self.curve == other.curve
        assert self.x.mod == other.x.mod

    def __add__(self, other):
        self.pointArithAssert(other)
        if self == other:
            return self.double()
        l = (other.y - self.y) // (other.x - self.x)
        return self.intersect(other, l)

    def intersect(self, other, l: ModNum):
        x = l * l - self.x - other.x
        y = l * (self.x - x) - self.y
        return EPoint(x, y, self.curve)

    def double(self):
        l = (3 * self.x ** 2 + self.curve.A) // (2 * self.y)
        return self.intersect(self, l)

    def compact(self) -> bytes:
        return self.x.val.to_bytes(7, 'little') + (self.y.val & 1).to_bytes(1, 'little')

    @staticmethod
    def from_compact(compact: bytes, curve: ECurve, mod: int):
        assert len(compact) == 8
        x = ModNum(int.from_bytes(compact[:7], 'little'), mod)
        y_parity = compact[7] & 1
        y_squared = x ** 3 + curve.A * x + curve.B
        y_val = pow(y_squared.val, (x.mod + 1) // 4, mod)
        y = ModNum(y_val, x.mod)
        assert y * y == y_squared
        if y is None:
            raise ValueError("Invalid point")
        if (y.val & 1) != y_parity:
            y = -y
        return EPoint(x, y, curve)

    def __mul__(self, times):
        assert isinstance(times, int) and times > 0
        if times == 1:
            return EPoint(ModNum(self.x.val, self.x.mod), ModNum(self.y.val, self.y.mod), self.curve)
        sq = self * (times // 2)
        sq = sq.double()
        if times % 2:
            return self + sq
        else:
            return sq

    def __rmul__(self, times):
        return self * times

    def __eq__(self, other):
        assert isinstance(other, EPoint)
        return self.x == other.x and self.y == other.y and self.curve == other.curve

    def __repr__(self):
        return f'({self.x.val}, {self.y.val})'

def mysecp():
    A = 0x5e924cd447a56b
    B = 0x892f0a953f589b
    curve = ECurve(A, B)
    mod = 0xbcffb098340493
    x = ModNum(0x9a77dc33b36acc, mod)
    y = ModNum(0x279be90a95dbdd, mod)
    assert y * y == x * x * x + A * x + B
    G = EPoint(x, y, curve)
    return curve, G, 0xbcffb09c43733d

class Signature:
    def __init__(self, r: ModNum, s: ModNum):
        assert isinstance(r, ModNum)
        assert isinstance(s, ModNum)
        self.r = r
        self.s = s

    def compact(self):
        r_size = (self.r.mod.bit_length() + 7) // 8
        s_size = (self.s.mod.bit_length() + 7) // 8
        return self.r.val.to_bytes(r_size, 'little') + self.s.val.to_bytes(s_size, 'little')

class ECC:
    def __init__(self, curve: ECurve, G: EPoint, order: int, d: int):
        self.curve = curve
        self.G = G
        self.order = order
        self.d = d


    def sign(self, message: bytes):
        z = int.from_bytes(hashlib.sha3_256(message).digest()[:8], 'little')
        n = self.order
        r, s = 0, 0
        while s == 0:
            while r == 0:
                k = random.randint(1, n - 1)
                P = k * self.G
                r = ModNum(P.x.val, n)
            s = (z + r * self.d) // k
        return Signature(r, s)

def gen_key(server_priv_key, parity):
    if parity not in (0, 1):
        raise ValueError("Parity must be 0 or 1")
    curve, G, order = mysecp()
    server = ECC(curve, G, order, server_priv_key)
    while True:
        privkey = random.randint(1, order - 1)
        pubkey = privkey * G
        if (pubkey.y.val & 1) == parity:
            break
    privkey_size = (order.bit_length() + 7 ) // 8
    privkey_bytes = privkey.to_bytes(privkey_size, 'little')
    pub_key_cert = server.sign(pubkey.compact()).compact()
    return privkey_bytes, pub_key_cert