// ============================================================================
// VAULT CHALLENGE (disposable heap-UAF CTF app).
// Object model, invariants, and the intended exploit: see VaultApp_design.md.
//
// TO REMOVE THIS FEATURE ENTIRELY:
//   1. Delete App/VaultApp.h, App/VaultApp.cc, App/VaultCommon.h.
//   2. Revert the "VAULT CHALLENGE" hooks in Hitcon.cpp and BadgeController.cc.
//   3. Keep the 4 recv ids in Logic/XBoardRecvFn.h.
//
// SERVER BUILD: build the station badge with BADGE_ROLE 0x10 (defines
// VAULT_SERVER; fill the real flags in that branch below), then flash that one
// badge. Every other build ships as Client with dummy flags, so reversing a
// player's own badge reveals no real challenge content.
// ============================================================================

#include "VaultApp.h"

#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Service/Sched/Scheduler.h>
#include <Util/callback.h>
#include <Util/uint_to_str.h>

#include <cstring>

#include "VaultCommon.h"

// BADGE_ROLE reaches here via VaultApp.h -> Hitcon.h -> Config/Custom.h.
#if BADGE_ROLE == 0x10
#define VAULT_SERVER
#endif

using namespace hitcon::service::xboard;
using hitcon::service::sched::task_callback_t;

namespace hitcon {
namespace app {
namespace vault {

// The #ifdef only picks the initial value; g_vault_role stays a non-const
// runtime global.
#ifdef VAULT_SERVER
VaultRole g_vault_role = VaultRole::Server;
#else
VaultRole g_vault_role = VaultRole::Client;
#endif

namespace {
#ifdef VAULT_SERVER
// flag len <= kSlotSize - 2
const char kFlag0[] = "FLAG{not_this_one}";
const char kFlag1[] = "FLAG{try_another}";
const char kFlag2[] = "FLAG{H4CK_M30W^._.^}";
#else
const char kFlag0[] = "FLAG{dummy1}";
const char kFlag1[] = "FLAG{dummy2}";
const char kFlag2[] = "FLAG{dummy3}";
#endif
constexpr uint16_t kKeyPrice[kNumItems] = {30, 30, 999};
// ---------------------------------------------------------------------------

// Navigation is self-managed; the base MenuApp just needs a non-null list.
const menu_entry_t kDummyMenu[] = {{"", nullptr, nullptr}};
const char* const kMainLabels[] = {"Store", "Inventory", "Name Upload", "Print",
                                   "Pinned"};
constexpr int kPageStep = 16;
// Server re-announces on this cadence so a client that missed the connect-time
// hello still joins the vault.
constexpr int kHelloPrio = 900;  // background task (Scheduler.h: prio 800-1000)
constexpr int kHelloIntervalMs = 500;
}  // namespace

VaultApp vault_app;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
VaultApp::VaultApp()
    : MenuApp(kDummyMenu, 1),
      hello_task_(kHelloPrio, (task_callback_t)&VaultApp::HelloTick, this,
                  kHelloIntervalMs) {
  hitcon::service::sched::scheduler.Queue(&hello_task_, nullptr);
  free_top_ = 0;
  money_ = 0;
  owned_count_ = 0;
  pinned_ = {};
  profile_slot_ = nullptr;
  for (int i = 0; i < kNumItems; ++i) bought_[i] = false;
  screen_ = Screen::Main;
  cursor_ = 0;
  action_ref_ = {};
  action_pin_only_ = false;
  name_pending_ = false;
  view_str_ = nullptr;
  nav_depth_ = 0;
}
#pragma GCC diagnostic pop

void VaultApp::Init() {
  g_xboard_logic.SetOnPacketArrive(CB_CAST(&VaultApp::OnHelloPacket), this,
                                   VAULT_HELLO_ID);
  g_xboard_logic.SetOnPacketArrive(CB_CAST(&VaultApp::OnBtnFwdPacket), this,
                                   VAULT_BTN_FWD_ID);
  g_xboard_logic.SetOnPacketArrive(CB_CAST(&VaultApp::OnNameReqPacket), this,
                                   VAULT_NAME_REQ_ID);
  g_xboard_logic.SetOnPacketArrive(CB_CAST(&VaultApp::OnNamePacket), this,
                                   VAULT_NAME_RESP_ID);
}

// ---------------------------- private allocator ----------------------------
// Free-list: a slot is free iff its index is on free_stack_.

Slot* VaultApp::Alloc() {
  if (free_top_ <= 0) return nullptr;
  int idx = free_stack_[--free_top_];
  return &pool_[idx];
}

void VaultApp::Free(Slot* s) {
  int idx = static_cast<int>(s - pool_);
  if (idx < 0 || idx >= kSlots) return;
  if (free_top_ >= kSlots) return;
  free_stack_[free_top_++] = idx;
}

// ------------------------------ server game --------------------------------

void VaultApp::ResetGame() {
  // Fill the free list so the first allocs hand out pool[0], pool[1], pool[2].
  free_top_ = kSlots;
  for (int i = 0; i < kSlots; ++i) free_stack_[i] = kSlots - 1 - i;

  money_ = 100;
  owned_count_ = 0;
  pinned_ = {};
  profile_slot_ = nullptr;
  for (int i = 0; i < kNumItems; ++i) bought_[i] = false;
  screen_ = Screen::Main;
  cursor_ = 0;
  action_ref_ = {};
  action_pin_only_ = false;
  name_pending_ = false;
  view_str_ = nullptr;
  nav_depth_ = 0;

  const char* flags[kNumItems] = {kFlag0, kFlag1, kFlag2};
  for (int i = 0; i < kNumItems; ++i) {
    Slot* s = Alloc();  // pool[0], pool[1], pool[2]
    s->locked.id = static_cast<uint8_t>(i);
    std::strncpy(s->locked.content, flags[i], kSlotSize - 1);
    s->locked.content[kSlotSize - 2] = '\0';
    owned_[owned_count_++] = {Type::Locked, s};
  }
}

int VaultApp::RemoveOwnedByPtr(Slot* p) {
  for (int i = 0; i < owned_count_; ++i) {
    if (owned_[i].ptr == p) {
      for (int j = i; j < owned_count_ - 1; ++j) owned_[j] = owned_[j + 1];
      owned_count_--;
      return i;
    }
  }
  return -1;
}

void VaultApp::BuyKey(int id) {
  if (id < 0 || id >= kNumItems) return;
  if (bought_[id]) return;
  if (money_ < kKeyPrice[id]) return;  // key#2 costs 999 -> unaffordable
  Slot* s = Alloc();
  if (!s) return;
  money_ -= kKeyPrice[id];
  s->key.id = static_cast<uint8_t>(id);
  s->key.used = 0;
  std::strncpy(s->key.desc, "key", sizeof(s->key.desc));
  owned_[owned_count_++] = {Type::Key, s};
  bought_[id] = true;  // single purchase -> drops out of the store list
}

void VaultApp::Use(Ref ref) {
  // Keys only. Plain "use" (reveal content) is handled by the Action dispatch,
  // which routes it through the Print screen so all display stays in Render().
  if (ref.ptr == nullptr || ref.type != Type::Key) return;

  Key* k = &ref.ptr->key;
  if (k->used) return;
  k->used = 1;

  int t = k->id;
  int li = -1;
  for (int i = 0; i < owned_count_; ++i) {
    if (owned_[i].type == Type::Locked && owned_[i].ptr->locked.id == t) {
      li = i;
      break;
    }
  }
  if (li >= 0) {
    Slot* lp = owned_[li].ptr;
    Slot* kp = ref.ptr;
    Slot* c = Alloc();
    if (c) {
      c->plain.id = static_cast<uint8_t>(t);
      std::memcpy(c->plain.content, lp->locked.content, kSlotSize - 1);
      owned_[owned_count_++] = {Type::Plain, c};
    }
    RemoveOwnedByPtr(kp);
    Free(kp);
    if (pinned_.ptr == kp) pinned_ = {};
    RemoveOwnedByPtr(lp);
    Free(lp);
  }
}

void VaultApp::Pin(Ref ref) { pinned_ = ref; }  // single slot; later pin wins

void VaultApp::Discard(Ref ref) {
  if (ref.ptr == nullptr) return;
  RemoveOwnedByPtr(ref.ptr);
  Free(ref.ptr);
}

void VaultApp::RequestName() {
  uint8_t marker = 0;
  g_xboard_logic.QueueDataForTx(&marker, 1, VAULT_NAME_REQ_ID);
}

// -------------------------------- rendering --------------------------------

int VaultApp::UnboughtCount() {
  int n = 0;
  for (int i = 0; i < kNumItems; ++i)
    if (!bought_[i]) n++;
  return n;
}

int VaultApp::StoreKeyIdAt(int row) {
  int want = row - 1;  // row 0 is the money line
  if (want < 0) return -1;
  int seen = 0;
  for (int id = 0; id < kNumItems; ++id) {
    if (bought_[id]) continue;
    if (seen == want) return id;
    seen++;
  }
  return -1;
}

bool VaultApp::ActionHasUse() {
  return action_ref_.type == Type::Key || action_ref_.type == Type::Plain;
}

int VaultApp::CurrentCount() {
  switch (screen_) {
    case Screen::Main:
      return 4 + (pinned_.ptr != nullptr ? 1 : 0);
    case Screen::Store:
      return 1 + UnboughtCount();
    case Screen::Inventory:
      return owned_count_;
    case Screen::Action: {
      int c = 0;
      if (ActionHasUse()) c++;  // Use (Key/Plain only)
      if (!action_pin_only_) {  // Pinned shortcut = Use only
        if (action_ref_.type != Type::Locked) c++;  // Discard (not on Locked)
        c++;                                        // Pin
      }
      return c;
    }
    case Screen::Print: {
      // cursor_ is the page index; count = number of pages of view_str_.
      if (view_str_ == nullptr) return 1;
      int len = static_cast<int>(std::strlen(view_str_));
      int pages = (len + kPageStep - 1) / kPageStep;
      return pages < 1 ? 1 : pages;
    }
    default:
      return 1;
  }
}

void VaultApp::BuildLabel(int idx) {
  int n = 0;
  switch (screen_) {
    case Screen::Main:
      std::strcpy(line_, kMainLabels[idx]);
      return;
    case Screen::Store:
      if (idx == 0) {
        line_[0] = '$';
        uint_to_chr(&line_[1], sizeof(line_) - 1,
                    static_cast<unsigned>(money_ < 0 ? 0 : money_));
      } else {
        int id = StoreKeyIdAt(idx);
        std::strcpy(line_, "Buy K");
        n = 5;
        n += uint_to_chr(&line_[n], sizeof(line_) - n,
                         static_cast<unsigned>(id));
        line_[n++] = ':';
        uint_to_chr(&line_[n], sizeof(line_) - n, kKeyPrice[id]);
      }
      return;
    case Screen::Inventory: {
      Ref r = owned_[idx];
      if (pinned_.ptr != nullptr && pinned_.ptr == r.ptr) line_[n++] = '*';
      unsigned num = 0;
      switch (r.type) {
        case Type::Key:
          std::strcpy(&line_[n], "Key>");
          n += 4;
          num = r.ptr->key.id;
          break;
        case Type::Locked:
          std::strcpy(&line_[n], "Lock#");
          n += 5;
          num = r.ptr->locked.id;
          break;
        case Type::Plain:
          std::strcpy(&line_[n], "Plain#");
          n += 6;
          num = r.ptr->plain.id;
          break;
        default:
          line_[n] = '\0';
          return;
      }
      uint_to_chr(&line_[n], sizeof(line_) - n, num);
      return;
    }
    case Screen::Action: {
      const char* opts[3];
      int cnt = 0;
      if (ActionHasUse()) opts[cnt++] = "Use";
      if (!action_pin_only_) {  // Pinned shortcut = Use only
        if (action_ref_.type != Type::Locked) opts[cnt++] = "Discard";
        opts[cnt++] = "Pin";
      }
      std::strcpy(line_, opts[idx]);
      return;
    }
    case Screen::NameReq:
      if (name_pending_) {
        std::strcpy(line_, "Uploading...");
      } else {
        std::strncpy(line_, "Done", sizeof(line_) - 1);
        line_[sizeof(line_) - 1] = '\0';
      }
      return;
    default:
      line_[0] = '\0';
      return;
  }
}

void VaultApp::Render() {
  int c = CurrentCount();
  if (c <= 0) {
    display_set_mode_scroll_text("(empty)");
    return;
  }
  if (cursor_ >= c) cursor_ = c - 1;
  if (cursor_ < 0) cursor_ = 0;
  if (screen_ == Screen::Print) {
    if (view_str_ == nullptr) {
      display_set_mode_scroll_text("");
      return;
    }
    // one page of view_str_ (cursor_ = page index)
    display_set_mode_scroll_text(view_str_ + cursor_ * kPageStep);
    return;
  }
  BuildLabel(cursor_);
  display_set_mode_scroll_text(line_);
}

void VaultApp::PushNav() {
  if (nav_depth_ < 4) {
    nav_screen_[nav_depth_] = screen_;
    nav_cursor_[nav_depth_] = cursor_;
    nav_depth_++;
  }
}

void VaultApp::Descend(Screen s) {
  PushNav();
  screen_ = s;
  cursor_ = 0;
  Render();
}

void VaultApp::Ascend() {
  if (nav_depth_ > 0) {
    nav_depth_--;
    screen_ = nav_screen_[nav_depth_];
    cursor_ = nav_cursor_[nav_depth_];  // restore the parent's position
  } else {
    screen_ = Screen::Main;
    cursor_ = 0;
  }
  Render();
}

void VaultApp::EnterAction(Ref ref, bool pin_only) {
  if (ref.ptr == nullptr) return;
  PushNav();
  action_ref_ = ref;
  action_pin_only_ = pin_only;  // Pinned-shortcut entry -> only "Use"
  screen_ = Screen::Action;
  cursor_ = 0;
  Render();
}

// ------------------------------ button routing -----------------------------

void VaultApp::HandleButton(button_t button) {
  if (g_vault_role != VaultRole::Server) return;

  if (button == BUTTON_UP || button == BUTTON_DOWN) {
    int c = CurrentCount();
    if (c <= 0) return;
    if (button == BUTTON_UP)
      cursor_ = (cursor_ == 0) ? c - 1 : cursor_ - 1;
    else
      cursor_ = (cursor_ + 1 >= c) ? 0 : cursor_ + 1;
    Render();
    return;
  }

  if (button == BUTTON_BACK) {
    if (screen_ != Screen::Main) Ascend();  // restore parent at its cursor
    return;
  }

  if (button != BUTTON_OK) return;

  switch (screen_) {
    case Screen::Main:
      switch (cursor_) {
        case 0:
          Descend(Screen::Store);
          break;
        case 1:
          Descend(Screen::Inventory);
          break;
        case 2:
          name_pending_ = true;
          RequestName();
          Descend(Screen::NameReq);  // waiting screen until the reply arrives
          break;
        case 3:
          view_str_ = profile_slot_ ? profile_slot_->profile.name : "Guest";
          Descend(Screen::Print);
          break;
        case 4:
          EnterAction(pinned_, true);  // Pinned shortcut -> Use only
          break;
        default:
          break;
      }
      break;

    case Screen::Store:
      if (cursor_ >= 1) {
        int id = StoreKeyIdAt(cursor_);
        if (id >= 0) BuyKey(id);
        Render();  // bought row dropped -> re-render current row
      }
      break;

    case Screen::Inventory:
      if (owned_count_ > 0) EnterAction(owned_[cursor_], false);
      break;

    case Screen::Action: {
      // Walk the same option list as BuildLabel: [Use?] then, unless pin-only,
      // [Discard?][Pin].
      int idx = cursor_;
      if (ActionHasUse()) {
        if (idx == 0) {
          if (action_ref_.type == Type::Plain) {
            view_str_ =
                action_ref_.ptr->plain.content;  // view via Print screen
            Descend(Screen::Print);
          } else {  // Key: apply, then back to source
            Use(action_ref_);
            Ascend();
          }
          break;
        }
        idx--;
      }
      if (!action_pin_only_) {                   // Pinned shortcut = Use only
        if (action_ref_.type != Type::Locked) {  // Discard
          if (idx == 0) {
            Discard(action_ref_);
            Ascend();
            break;
          }
          idx--;
        }
        Pin(action_ref_);
        Ascend();
      }
      break;
    }

    case Screen::Print: {
      int c = CurrentCount();
      cursor_ = (cursor_ + 1 >= c) ? 0 : cursor_ + 1;  // Enter -> next page
      Render();
      break;
    }

    default:
      break;
  }
}

// ------------------------------ App interface ------------------------------

void VaultApp::OnEntry() {
  if (g_vault_role == VaultRole::Server) {
    display_set_orientation(0);
    ResetGame();
    hitcon::service::sched::scheduler.EnablePeriodic(&hello_task_);
    active = true;
    Render();
  } else {
    active = false;
    display_set_mode_scroll_text("Controlling...");
  }
}

void VaultApp::OnExit() {
  if (g_vault_role == VaultRole::Server) {
    display_set_orientation(1);
    hitcon::service::sched::scheduler.DisablePeriodic(&hello_task_);
  }
  active = false;
}

void VaultApp::OnButton(button_t button) {
  // Client: forward every button to the server. Server: local buttons do
  // nothing; it is driven only by forwarded packets.
  if (g_vault_role == VaultRole::Client) {
    VaultBtnPacket pkt{static_cast<uint16_t>(button)};
    g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt),
                                  VAULT_BTN_FWD_ID);
  }
}

void VaultApp::SendHello() {
  uint8_t marker = 0;
  g_xboard_logic.QueueDataForTx(&marker, 1, VAULT_HELLO_ID);
}

void VaultApp::HelloTick(void* arg) {
  (void)arg;
  SendHello();
}

// ----------------------------- packet handlers -----------------------------

void VaultApp::OnHelloPacket(void* arg) {
  (void)arg;
  if (g_vault_role != VaultRole::Client) return;
  if (g_xboard_logic.GetPeer() != PeerType::Peer2025) return;
  if (badge_controller.GetCurrentApp() != this) {
    badge_controller.change_app(this);
  }
}

void VaultApp::OnBtnFwdPacket(void* arg) {
  if (g_vault_role != VaultRole::Server) return;
  auto* cb = static_cast<PacketCallbackArg*>(arg);
  if (cb->len != sizeof(VaultBtnPacket)) return;
  VaultBtnPacket pkt;
  std::memcpy(&pkt, cb->data, sizeof(pkt));
  HandleButton(static_cast<button_t>(pkt.button));
}

void VaultApp::OnNameReqPacket(void* arg) {
  (void)arg;
  if (g_vault_role != VaultRole::Client) return;
  if (g_xboard_logic.GetPeer() != PeerType::Peer2025) return;
  VaultNamePacket pkt;
  std::strncpy(pkt.name, show_name_app.name, kVaultNameBufLen);
  pkt.name[kVaultNameBufLen - 1] = '\0';
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt),
                                VAULT_NAME_RESP_ID);
}

void VaultApp::OnNamePacket(void* arg) {
  if (g_vault_role != VaultRole::Server) return;
  auto* cb = static_cast<PacketCallbackArg*>(arg);
  if (cb->len != sizeof(VaultNamePacket)) return;
  VaultNamePacket pkt;
  std::memcpy(&pkt, cb->data, sizeof(pkt));
  pkt.name[kVaultNameBufLen - 1] = '\0';

  if (profile_slot_ != nullptr) {
    Free(profile_slot_);
    profile_slot_ = nullptr;
  }
  Slot* s = Alloc();
  if (!s) return;
  s->profile.id = 0xFF;
  std::strncpy(s->profile.name, pkt.name, kSlotSize - 1);
  profile_slot_ = s;
  name_pending_ = false;
  if (screen_ == Screen::NameReq) Render();  // player is watching -> show name
}

}  // namespace vault
}  // namespace app
}  // namespace hitcon
