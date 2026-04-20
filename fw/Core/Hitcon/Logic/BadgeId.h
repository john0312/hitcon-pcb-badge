#ifndef HITCON_LOGIC_BADGE_ID_H_
#define HITCON_LOGIC_BADGE_ID_H_

#include <Service/EcParams.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

// Currently we set the badge ID to 4 bytes taken from the compact public key:
// bytes 3-6. Might switch to a hash of the public key if there are concerns
// about collisions.
constexpr size_t BADGE_ID_LEN = 4;

/**
 * Returns a pointer to the badge ID with BADGE_ID_LEN bytes.
 * Returns nullptr if public key is not ready yet.
 * Note that the returned buffer may no longer be valid after the current task
 * ends.
 */
const uint8_t *GetBadgeId();

/**
 * Copies badge ID into ptr. Buffer should be at least BADGE_ID_LEN.
 * This function does not perform any size checks! Caller is expected to do so.
 * Returns false and zeros the buffer if badge ID is not ready.
 */
bool SetBufferToBadgeId(uint8_t *ptr);

}  // namespace hitcon

#endif  // HITCON_LOGIC_BADGE_ID_H_
