#include <Logic/BadgeId.h>
#include <Logic/EcLogic.h>

#include <cstring>

namespace hitcon {

const uint8_t *GetBadgeId() {
  // To guarantee maximum entropy, we use the last 4 bytes of the x-coordinate
  // of the public key, which is byte 3 - 6 inclusive.
  const uint8_t *ptr = hitcon::ecc::g_ec_logic.GetPublicKey();
  if (!ptr) return ptr;
  ptr += (ECC_PUBKEY_SIZE - BADGE_ID_LEN - 1);
  return ptr;
}

bool SetBufferToBadgeId(uint8_t *ptr) {
  auto *badge_id = GetBadgeId();
  if (!badge_id) {
    // Not ready.
    memset(ptr, 0, BADGE_ID_LEN);
    return false;
  }
  memcpy(ptr, badge_id, BADGE_ID_LEN);
  return true;
}

}  // namespace hitcon
