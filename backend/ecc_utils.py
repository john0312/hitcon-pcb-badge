import ecc
from schemas import EccPoint, EccPublicKey, EccPrivateKey, EccSignature

ECC_SIGNATURE_SIZE = 14
ECC_PUBKEY_SIZE = 8

curve, G, order = ecc.mysecp()

def ecc_sign(msg: bytes, priv: EccPrivateKey) -> EccSignature:
    # Sign the given bytes with the private key.
    ECC_instance = ecc.ECC(curve, G, order, priv.dA)
    r, s = ECC_instance.sign(message=msg)
    return EccSignature(r=r.val, s=s.val, pub=ecc_derive_pub(priv))


def ecc_verify(msg: bytes, sig: EccSignature) -> bool:
    # Verify the given signature. sig.pub must be populated.
    assert sig.pub is not None, "Public key must be provided for signature verification"

    mod = G.x.mod

    ECC_instance = ecc.ECC(curve, G, order, 0)
    return ECC_instance.verify(
        message=msg,
        pub=ecc.EPoint(
            x=ecc.ModNum(sig.pub.point.x, mod),
            y=ecc.ModNum(sig.pub.point.y, mod),
            curve=curve
        ),
        r=ecc.ModNum(sig.r, order),
        s=ecc.ModNum(sig.s, order)
    )


def ecc_derive_pub(priv: EccPrivateKey) -> EccPublicKey:
    # Derive the public key from the private key.
    ECC_instance = ecc.ECC(curve, G, order, priv.dA)
    point = ECC_instance.pub()

    return EccPublicKey(point=EccPoint(x=point.x.val, y=point.y.val))


def ecc_get_point_by_x(x: int, sign: bool = False) -> EccPoint:
    # Get the y-coordinate for the given x-coordinate on the curve.
    # This is used to verify the point is on this specific curve.
    mod = G.x.mod
    y_squared = (x ** 3 + x * curve.A + curve.B) % mod
    y = modulus_congruent_to_3_module_4(y_squared, mod)

    if int(sign) ^ (y & 1):
        y = -y

    y = y % mod

    return EccPoint(x=x, y=y)


def modulus_congruent_to_3_module_4(a: int, m: int)-> int:
    # https://www.rieselprime.de/ziki/Modular_square_root#Modulus_congruent_to_3_modulo_4
    return pow(a, (m + 1) // 4, m)