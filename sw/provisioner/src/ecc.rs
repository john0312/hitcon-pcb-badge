//! Private-key generation for badge provisioning.
//!
//! Port of pcb-util/dev/ecc.py `gen_key`: a private key is a random scalar `d`
//! on a custom ~56-bit short-Weierstrass curve, retried until the public key
//! `d*G` has the requested y-parity. Because the field is only ~56 bits every
//! product fits in u128, so no bignum library is needed.

use rand::RngExt;

// Curve y^2 = x^3 + A*x + B (mod P); generator G, group order N.
// Values match pcb-util/dev/ecc.py `mysecp`.
const A: u128 = 0x5e924cd447a56b;
// Curve coefficient B: only the on-curve test references it; the add/double formulas need A only.
#[allow(dead_code)]
const B: u128 = 0x892f0a953f589b;
const P: u128 = 0xbcffb098340493;
const N: u128 = 0xbcffb09c43733d;
const GX: u128 = 0x9a77dc33b36acc;
const GY: u128 = 0x279be90a95dbdd;
const G: Point = Point { x: GX, y: GY };

/// Private key length in bytes: ceil(bit_length(N) / 8) = ceil(56 / 8) = 7.
pub(crate) const PRIV_KEY_LEN: usize = 7;

// --- Field arithmetic mod P (mirrors ecc.py ModNum) ---

fn add(a: u128, b: u128) -> u128 {
    (a + b) % P
}

fn sub(a: u128, b: u128) -> u128 {
    // a + P - b avoids unsigned underflow; matches ModNum.__sub__.
    (a + P - b) % P
}

fn mul(a: u128, b: u128) -> u128 {
    // a, b < P < 2^56, so a*b < 2^112 fits in u128.
    (a * b) % P
}

/// Modular inverse of `a` mod P via the extended Euclidean algorithm.
/// Faithful port of ecc.py `gcdExtend(a, P)`; requires `a` coprime to P (P prime, a != 0).
fn inv(a: u128) -> u128 {
    let m = P as i128;
    let mut ppr = a as i128;
    let mut pr = P as i128;
    let mut ppx: i128 = 1;
    let mut px: i128 = 0;
    while pr != 1 {
        let q = ppr / pr;
        let r = ppr % pr;
        let x = ((ppx - q * px) % m + m) % m; // keep non-negative like Python's %
        ppr = pr;
        pr = r;
        ppx = px;
        px = x;
    }
    px as u128
}

/// a / b mod P (ModNum.__floordiv__).
fn div(a: u128, b: u128) -> u128 {
    mul(a, inv(b))
}

// --- Curve point arithmetic (affine; mirrors ecc.py EPoint) ---

#[derive(Clone, Copy, PartialEq, Eq)]
struct Point {
    x: u128,
    y: u128,
}

/// Third intersection point given slope l (ecc.py EPoint.intersect).
fn intersect(p: Point, q: Point, l: u128) -> Point {
    let x = sub(sub(mul(l, l), p.x), q.x);
    let y = sub(mul(l, sub(p.x, x)), p.y);
    Point { x, y }
}

/// Point doubling (ecc.py EPoint.double): l = (3*x^2 + A) / (2*y).
fn double(p: Point) -> Point {
    let l = div(add(mul(3, mul(p.x, p.x)), A), mul(2, p.y));
    intersect(p, p, l)
}

/// Point addition (ecc.py EPoint.__add__): doubles when the points are equal.
fn point_add(p: Point, q: Point) -> Point {
    if p == q {
        return double(p);
    }
    let l = div(sub(q.y, p.y), sub(q.x, p.x));
    intersect(p, q, l)
}

/// Scalar multiplication `times * p` by double-and-add.
/// Faithful port of ecc.py EPoint.__mul__ (recursive); requires `times >= 1`.
fn scalar_mul(times: u128, p: Point) -> Point {
    if times == 1 {
        return p;
    }
    let sq = double(scalar_mul(times / 2, p));
    if times % 2 == 1 { point_add(p, sq) } else { sq }
}

/// Generate a private key for the given team parity (BLUE = 0, RED = 1).
/// Retries fresh random scalars until the public key `d*G` has y-parity == `parity`.
/// Returns `d` as 7 bytes little-endian (matches ecc.py `gen_key`).
pub(crate) fn gen_key(parity: u8) -> [u8; PRIV_KEY_LEN] {
    assert!(parity == 0 || parity == 1, "parity must be 0 or 1");
    let mut rng = rand::rng();
    loop {
        let d = rng.random_range(1..N); // d in [1, N-1]
        let pubkey = scalar_mul(d, G);
        if (pubkey.y & 1) as u8 == parity {
            let le = d.to_le_bytes();
            let mut out = [0u8; PRIV_KEY_LEN];
            out.copy_from_slice(&le[..PRIV_KEY_LEN]);
            return out;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Whether a point satisfies y^2 == x^3 + A*x + B (mod P).
    fn on_curve(p: Point) -> bool {
        let lhs = mul(p.y, p.y);
        let rhs = add(add(mul(mul(p.x, p.x), p.x), mul(A, p.x)), B);
        lhs == rhs
    }

    // Reference vectors for d*G, computed with pcb-util/dev/ecc.py: (d, x, y).
    const VECTORS: &[(u128, u128, u128)] = &[
        (0x1, 0x9a77dc33b36acc, 0x279be90a95dbdd),
        (0x2, 0x9a2e8af9f0403c, 0x52b00e5f0b8ee1),
        (0x3, 0x2415048f5fa910, 0x2ae9d459887b45),
        (0x7, 0x12fd78df99ee27, 0x446b65155e5d77),
        (0x1234567890abcd, 0x27c9ab19481936, 0x894ad3c33393b1),
        (0xbcffb09c43733c, 0x9a77dc33b36acc, 0x9563c78d9e28b6), // (N-1)*G = -G
        (0xb3a73ce2ff2, 0x9c0795e840b788, 0x52bcfd7eb57491),
    ];

    #[test]
    fn generator_on_curve() {
        assert!(on_curve(G));
    }

    #[test]
    fn scalar_mul_matches_python() {
        for &(d, x, y) in VECTORS {
            let pt = scalar_mul(d, G);
            assert_eq!(pt.x, x, "x mismatch for d={d:#x}");
            assert_eq!(pt.y, y, "y mismatch for d={d:#x}");
            assert!(on_curve(pt), "off curve for d={d:#x}");
        }
    }

    #[test]
    fn priv_key_encoding_is_le7() {
        // d.to_le_bytes()[..7] must equal Python d.to_bytes(7, 'little').
        let d: u128 = 0x1234567890abcd;
        assert_eq!(
            &d.to_le_bytes()[..PRIV_KEY_LEN],
            &[0xcd, 0xab, 0x90, 0x78, 0x56, 0x34, 0x12]
        );
    }

    #[test]
    fn gen_key_respects_parity_and_range() {
        for parity in [0u8, 1u8] {
            let key = gen_key(parity);
            // Reconstruct d and confirm d*G has the requested parity.
            let mut buf = [0u8; 16];
            buf[..PRIV_KEY_LEN].copy_from_slice(&key);
            let d = u128::from_le_bytes(buf);
            assert!(d >= 1 && d < N, "d out of range for parity={parity}");
            let pubkey = scalar_mul(d, G);
            assert_eq!((pubkey.y & 1) as u8, parity, "parity mismatch");
        }
    }
}
