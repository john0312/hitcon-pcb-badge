#ifndef HITCON_SERVICE_PERBOARDDATA_H_
#define HITCON_SERVICE_PERBOARDDATA_H_

#include <Service/EcParams.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

class PerBoardData {
 public:
  constexpr PerBoardData();

  const uint8_t* GetPerBoardRandom();

  const uint8_t* GetPerBoardSecret();

  const uint8_t* GetPubKeyCert();

  static constexpr size_t kRandomLen = 16;
  static constexpr size_t kSecretLen = 16;
  static constexpr size_t kPubKeyCertSize = hitcon::ECC_SIGNATURE_SIZE;
};

extern PerBoardData g_per_board_data;
}  // namespace hitcon

#endif  // #ifndef HITCON_SERVICE_PERBOARDDATA_H_
