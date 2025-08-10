#ifndef SHA3_H
#define SHA3_H

// Taken from https://github.com/brainhub/SHA3IUF
// Slightly modified for this project.
// See license below.

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Works when compiled for either 32-bit or 64-bit targets, optimized for
 * 64 bit.
 *
 * Canonical implementation of Init/Update/Finalize for SHA-3 byte input.
 *
 * SHA3-256, SHA3-384, SHA-512 are implemented. SHA-224 can easily be added.
 *
 * Based on code from http://keccak.noekeon.org/ .
 *
 * I place the code that I wrote into public domain, free to use.
 *
 * I would appreciate if you give credits to this work if you used it to
 * write or test * your code.
 *
 * Aug 2015. Andrey Jivsov. crypto@brainhub.org
 * ---------------------------------------------------------------------- */

/* 'Words' here refers to uint64_t */
#define SHA3_KECCAK_SPONGE_WORDS \
  (((1600) / 8 /*bits to byte*/) / sizeof(uint64_t))
typedef struct sha3_context_ {
  uint64_t saved; /* the portion of the input message that we
                   * didn't consume yet */
  union {         /* Keccak's state */
    uint64_t s[SHA3_KECCAK_SPONGE_WORDS];
    uint8_t sb[SHA3_KECCAK_SPONGE_WORDS * 8];
  } u;
  unsigned byteIndex;     /* 0..7--the next byte after the set one
                           * (starts from 0; 0--none are buffered) */
  unsigned wordIndex;     /* 0..24--the next word to integrate input
                           * (starts from 0) */
  unsigned capacityWords; /* the double size of the hash output in
                           * words (e.g. 16 for Keccak 512) */
} sha3_context;

enum SHA3_FLAGS { SHA3_FLAGS_NONE = 0, SHA3_FLAGS_KECCAK = 1 };

enum SHA3_RETURN { SHA3_RETURN_OK = 0, SHA3_RETURN_BAD_PARAMS = 1 };
typedef enum SHA3_RETURN sha3_return_t;

/* For Init or Reset call these: */
sha3_return_t sha3_Init(void *priv, unsigned bitSize);

// Takes 16ms on STM32@12MHz, should be split.
void keccakf(uint64_t s[25]);
// Takes 0.7ms on STM32@12MHz, should only be called once per task.
// round should be sequentially called with [0, KECCAK_ROUNDS-1], for a total of
// KECCAK_ROUNDS times.
void keccakf_split(uint64_t s[25], int round);

void sha3_Init256(void *priv);
void sha3_Init384(void *priv);
void sha3_Init512(void *priv);

enum SHA3_FLAGS sha3_SetFlags(void *priv, enum SHA3_FLAGS);

// Takes up to 1 keccakf() call.
void sha3_UpdateWord(void *priv, void const *bufIn);

// Takes multiple keccakf() call and should not be used on STM32.
void sha3_Update(void *priv, void const *bufIn, size_t len);

// Takes 1 keccakf() call.
void const *sha3_Finalize(void *priv);

// This is exactly the same as sha3_Finalize() but the caller is expected
// to call this KECCAK_ROUNDS+2 times, each time giving round = 0 to
// KECCAK_ROUNDS+1.
// For round=0, it should handle anything before keccakf().
// For round=1 to KECCAK_ROUNDS, it should call keccakf(round-1).
// For round=KECCAK_ROUNDS+1, it should handle anything after keccakf().
void const *sha3_Finalize_split(void *priv, int round);

/* Single-call hashing */
sha3_return_t sha3_HashBuffer(
    unsigned bitSize,      /* 256, 384, 512 */
    enum SHA3_FLAGS flags, /* SHA3_FLAGS_NONE or SHA3_FLAGS_KECCAK */
    const void *in, unsigned inBytes, void *out,
    unsigned outBytes); /* up to bitSize/8; truncation OK */

constexpr size_t SHA3_256_HASH_SIZE = 256 / 8;
constexpr size_t KECCAK_ROUNDS = 24;

#endif
