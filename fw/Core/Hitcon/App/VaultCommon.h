#ifndef HITCON_APP_VAULT_COMMON_H_
#define HITCON_APP_VAULT_COMMON_H_

// VAULT CHALLENGE (disposable) -- xboard wire structs shared by both roles.
// See removal notes at the top of VaultApp.cc.

#include <App/ShowNameApp.h>

#include <cstddef>
#include <cstdint>

namespace hitcon {
namespace app {
namespace vault {

// Wire buffer for a name: NAME_LEN chars + null. Packet size only, independent
// of the pool slot size (kSlotSize in VaultApp.h, which adds a leading id
// byte).
constexpr size_t kVaultNameBufLen = ShowNameApp::NAME_LEN + 1;  // 23

// client -> server: forwarded button press
struct VaultBtnPacket {
  uint16_t button;
} __attribute__((packed));

// client -> server: the client's own badge name
struct VaultNamePacket {
  char name[kVaultNameBufLen];
} __attribute__((packed));

// VAULT_HELLO_ID (server->client) and VAULT_NAME_REQ_ID (server->client) carry
// a single ignored marker byte.

}  // namespace vault
}  // namespace app
}  // namespace hitcon

#endif  // HITCON_APP_VAULT_COMMON_H_
