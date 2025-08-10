import hashlib

SHA3_PREFIX_LEN = 8

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


class ECC:
    def __init__(self, curve: ECurve, G: EPoint, order: int, d: int):
        self.curve = curve
        self.G = G
        self.order = order
        self.d = d

    def pub(self) -> EPoint:
        return self.d * self.G

    def sign(self, message: bytes):
        z = int.from_bytes(hashlib.sha3_256(message).digest()[:SHA3_PREFIX_LEN], 'little')
        n = self.order
        r, s = ModNum(0, n), ModNum(0, n)
        while s == 0:
            while r == 0:
                k = 2
                P = k * self.G
                r = ModNum(P.x.val, n)
            s = (z + r * self.d) // k
        return r, s

    def verify(self, message: bytes, pub: EPoint, r: ModNum, s: ModNum):
        n = self.order
        if not 1 <= r <= n-1 or not 1 <= s <= n-1:
            return False
        z = ModNum(int.from_bytes(hashlib.sha3_256(message).digest()[:SHA3_PREFIX_LEN], 'little'), r.mod)
        u1: ModNum = z // s
        u2: ModNum = r // s
        P: EPoint = u1.val * self.G + u2.val * pub
        return P.x.val == r.val