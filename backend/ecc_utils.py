import schemas

def ecc_sign(msg: bytes, priv: schemas.EccPrivateKey) -> schemas.EccSignature:
    # Sign the given bytes with the private key.
    # TODO
    pass

def ecc_verify(sig: schemas.EccSignature) -> bool:
    # Verify the given signature. sig.pub must be populated.
    # TODO
    pass

def ecc_derive_pub(priv: schemas.EccPrivateKey) -> schemas.EccPublicKey:
    # Derive the public key from the private key.
    pass

