#ifndef HITCON_APP_VAULT_APP_H_
#define HITCON_APP_VAULT_APP_H_

// VAULT CHALLENGE (disposable). Full removal notes are at the top of
// VaultApp.cc. This module is self-contained; no other file references its
// internals except the four marked hooks listed there.

#include <App/MenuApp.h>
#include <App/ShowNameApp.h>
#include <Hitcon.h>
#include <Service/Sched/PeriodicTask.h>

#include <cstdint>

namespace hitcon {
namespace app {
namespace vault {

// Runtime role (non-const on purpose). Defaults to Client; the server build
// flips the initial value to Server.
enum class VaultRole : uint8_t { Client, Server };
extern VaultRole g_vault_role;

// ---- pool object model (private allocator lives entirely in VaultApp.cc) ----
constexpr int kSlots = 8;
constexpr int kSlotSize = ShowNameApp::NAME_LEN + 2;  // id byte + name + null
constexpr int kNumItems = 3;

struct Profile {
  uint8_t id;
  char name[kSlotSize - 1];  // uploaded name
};
struct Key {
  uint8_t id;  // matches the Locked id it opens
  char desc[kSlotSize - 2];
  uint8_t used;
};
struct Locked {
  uint8_t id;
  char content[kSlotSize - 1];  // the flag
};
struct Plain {
  uint8_t id;
  char content[kSlotSize - 1];  // decrypted copy
};
union Slot {
  Profile profile;
  Key key;
  Locked locked;
  Plain plain;
};
static_assert(sizeof(Slot) == kSlotSize,
              "Slot must be exactly kSlotSize bytes");

// App-side type tag carried on Ref, never stored in a pool slot.
enum class Type : uint8_t { Key, Locked, Plain, Profile };
struct Ref {
  Type type;
  Slot* ptr;
};

// Hierarchical server screens (navigation is self-managed, not MenuApp's).
// NameReq shows one of two states via name_pending_: waiting for the reply, or
// the received name.
enum class Screen : uint8_t { Main, Store, Inventory, Action, Print, NameReq };

class VaultApp : public MenuApp {
 public:
  VaultApp();
  virtual ~VaultApp() = default;

  void Init();

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  // MenuApp pure-virtuals: unused (navigation handled internally).
  void OnButtonMode() override {}
  void OnButtonBack() override {}
  void OnButtonLongBack() override {}

  // Server announces itself on peer connect (called from BadgeController).
  void SendHello();

 private:
  // xboard packet handlers (non-virtual: required by CB_CAST).
  void OnHelloPacket(void* arg);
  void OnBtnFwdPacket(void* arg);
  void OnNameReqPacket(void* arg);
  void OnNamePacket(void* arg);
  void HelloTick(void* arg);  // periodic: server re-announces itself

  // private pool allocator
  Slot* Alloc();
  void Free(Slot* s);

  // server game
  void ResetGame();
  int RemoveOwnedByPtr(Slot* p);
  void BuyKey(int id);
  void Use(Ref ref);
  void Pin(Ref ref);
  void Discard(Ref ref);
  void RequestName();

  // screen state machine
  void HandleButton(button_t button);  // from a forwarded packet
  int CurrentCount();
  int UnboughtCount();
  int StoreKeyIdAt(int row);  // store row (>=1) -> key id, else -1
  bool ActionHasUse();
  void BuildLabel(int idx);  // -> line_
  void Render();
  void PushNav();  // remember (screen_, cursor_) before descending
  void Descend(Screen s);
  void Ascend();  // pop back to the parent screen at its saved cursor
  void EnterAction(Ref ref, bool pin_only);

  // pool + allocator
  Slot pool_[kSlots];
  uint8_t free_stack_[kSlots];
  int free_top_;

  // game state
  int money_;
  Ref owned_[kSlots];
  int owned_count_;
  Ref pinned_;
  Slot* profile_slot_;
  bool bought_[kNumItems];

  // ui state
  Screen screen_;
  int cursor_;
  Ref action_ref_;
  bool action_pin_only_;  // Action reached via the Pinned shortcut -> Use only
  bool name_pending_;     // NameReq screen: true = waiting, false = reply shown
  const char* view_str_;  // Print screen source string
  // nav stack: Descend pushes (screen,cursor); Ascend restores parent position
  Screen nav_screen_[4];
  int nav_cursor_[4];
  int nav_depth_;
  char line_[32];
  hitcon::service::sched::PeriodicTask hello_task_;
};

extern VaultApp vault_app;

}  // namespace vault
}  // namespace app
}  // namespace hitcon

#endif  // HITCON_APP_VAULT_APP_H_
