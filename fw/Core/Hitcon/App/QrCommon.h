#ifndef HITCON_APP_QR_COMMON_H_
#define HITCON_APP_QR_COMMON_H_

#include <cstdint>

namespace hitcon {
namespace app {
namespace qr {

// client -> station
struct QrStnBtnPacket {
  uint16_t button;
} __attribute__((packed));

// station -> client
struct QrUnlockPacket {
  uint8_t station_id;
} __attribute__((packed));

}  // namespace qr
}  // namespace app
}  // namespace hitcon

#endif  // HITCON_APP_QR_COMMON_H_
