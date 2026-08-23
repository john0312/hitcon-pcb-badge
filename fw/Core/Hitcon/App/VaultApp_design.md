# VaultApp — design, invariants, and intended exploit

Internal author notes for the Vault heap-exploitation challenge. This is kept
out of the source comments on purpose: reading the code and solving the
challenge are separate concerns, and the comments should not spoil the solve.

## Overview

Vault-themed heap-exploitation CTF app. The player's badge (client) remote-
controls the server badge over the xboard connector by forwarding every button
press; the server runs the whole game and drives its own display. All logic
ships in every badge so a player can reverse their own board, but the real flag
exists only in the server build.

## Roles & startup

- Single `VaultApp`; the role is chosen at runtime by the non-const global
  `g_vault_role` (default `Client`). The server build flips the initial value to
  `Server` (`BADGE_ROLE == 0x10` defines `VAULT_SERVER`). Both role code paths
  stay in every binary, so the app is fully reverse-engineerable.
- On xboard peer connect the server enters the vault and re-sends `VAULT_HELLO`
  periodically (a `PeriodicTask` enabled in `OnEntry`, disabled in `OnExit`); a
  client switches into the vault on any hello. The periodic resend recovers the
  case where the client missed the first hello and would otherwise sit on
  `connect_menu`; `OnHelloPacket` is idempotent (already in the vault → no-op).
  Two clients connecting (both `Client`) send no hello and stay on the normal
  `connect_menu` — zero interference.

## Memory model (private pool, lives only in VaultApp.cc)

- `kSlots = 8` fixed-size slots; `kSlotSize = NAME_LEN + 2 = 24`.
- `union Slot { Profile; Key; Locked; Plain; }` — every member begins with a
  1-byte `id` at offset 0, so the "id" byte overlaps across all interpretations.
  - `Profile { u8 id; char name[23]; }` — `name` occupies offsets 1..23. `id` is
    a fixed header (see invariant 3).
  - `Key { u8 id; char desc[22]; u8 used; }` — `id` = the `Locked` id this key
    opens; `used` is the **last** slot byte (offset 23).
  - `Locked { u8 id; char content[23]; }` — the (fake/real) flag.
  - `Plain  { u8 id; char content[23]; }` — decrypted copy shown to the player.
- Allocator: LIFO free-list `free_stack_`; `Alloc` pops, `Free` pushes. This
  metadata is stored separately from `pool_`, so a leak that reads slot bytes
  never trips over allocator bookkeeping.
- The app-side object type is carried on `Ref { Type; Slot* }`, never stored in
  the slot. A leak therefore reads pure object bytes, and a `Ref` can end up
  describing a slot as a type it no longer holds.

## Load-bearing invariants — do NOT "clean these up", they ARE the challenge

1. `Free()` never scrubs the slot bytes and no code re-validates a pointer's
   liveness. Freed data lingers and dangling pointers stay usable → UAF.
2. `Discard()` frees the slot but intentionally does **not** touch `pinned_`.
   If the pin referenced that slot it now dangles. This single dangling-pin
   lifetime bug is the entire vulnerability.
3. `OnNamePacket` sets `profile.id = 0xFF`, outside the item id range
   `0..kNumItems-1`. A stale key that reinterprets a `Profile` slot then reads a
   non-item id, matches no `Locked`, and `Use()` performs only its stray write —
   it never enters the unlock branch. Because `name` starts at offset 1, the
   uploaded text cannot alias `id`, so the player cannot steer which item a stale
   key would unlock. Dropping this or using an item id reintroduces an
   unintended solve (and, worse, the unlock branch would free the profile slot
   and clear the pin, corrupting pool state).
4. `Key.used` is the last slot byte (offset 23). A full 22-char uploaded name
   lands its NUL terminator on that same byte, so the stray write `used = 1`
   overwrites the NUL and the printed name over-reads into the next slot.
5. `ResetGame` reverse-fills the free-list so the three `Locked`s seed into
   `pool[0]`, `pool[1]`, `pool[2]` as id 0, 1, 2. The id-2 `Locked` (the real
   flag) is therefore adjacent to the slot the exploit reuses (pool[1]), which
   is what the over-read lands in.
6. In the legitimate unlock path, `Use()` frees the matched `Locked` **last** so
   that slot tops the free-list and is the next `Alloc()` — this is what makes
   the "unlock frees a slot, next buy/upload reuses it" chain deterministic.
7. Keys are single-purchase (`bought_[]`); the id-2 key costs 999.

## Intended exploit (runs on the server, driven by forwarded buttons)

Start: money 100; key prices `{30, 30, 999}`. The id-2 `Locked` @ pool[2] holds
the real flag; its key costs 999 (unaffordable).

1. **Store → Buy key(id1)** (30): allocates a `Key{id 1}` into inventory.
2. **Inventory → Use key(id1)**: unlocks the id-1 `Locked` @ pool[1]. This
   allocates a new `Plain` slot, copies the decrypted id-1 content into it, and
   adds it to inventory; then it frees the key's slot and the id-1 `Locked`.
   The `Locked` is freed **last**, so pool[1] ends up on top of the free-list.
3. **Store → Buy key(id0)** (30): the allocation reuses pool[1] as a `Key{id 0}`.
4. **Inventory → Pin key(id0)**: sets `pinned_ = {Key, pool[1]}`.
5. **Inventory → Discard key(id0)**: frees pool[1] and removes it from
   inventory; `pinned_` is left untouched, so it now dangles at pool[1].
6. **Main → Name Upload** with a **22-char** name: the allocation reuses pool[1]
   as a `Profile` (`id = 0xFF`, `name` fills offsets 1..22, NUL at offset 23).
7. **Main → Pinned → Use**: the type check passes (the `Ref` still records
   `Key`); `k->used = 1` writes offset 23, clobbering the name's NUL; `t =
   profile.id = 0xFF` matches no `Locked`, so nothing is unlocked — only the
   write happens.
8. **Main → Print**: `view_str_ = profile.name` (offset 1). With the NUL at
   offset 23 gone, `strlen` runs past pool[1] into pool[2] = `Locked{id 2, real
   flag}`. The display pages show: `<22 name chars>` + `0x01` (used) + `0x02`
   (id 2) + the real flag, stopping at the flag's NUL.

Why each piece is needed:

- The **pin** is mandatory: after discard the key is only reachable through
  `pinned_`, so the stale `Use` can only come from the Pinned shortcut.
- The two non-zero bytes between the name and the flag (`used`, then the id) keep
  `strlen` from stopping early.
- Pool contiguity (stride `kSlotSize = 24`) makes the over-read land exactly on
  the adjacent item slot.

## Why it can't be shortcut

- The id-2 key costs 999 → the id-2 `Locked` can't be unlocked legitimately.
- `profile.id = 0xFF` + `name` at offset 1 → the uploaded text can neither alias
  the id nor steer a stale key to unlock an item.
- `Use()`'s unlock branch fires at most once per key: the `used` guard blocks
  re-entry, and on firing the key is removed from inventory + freed and the pin
  is cleared, so there is no live handle to invoke it again.

## Build / deploy

- **Server**: build the station badge with `BADGE_ROLE 0x10` (defines
  `VAULT_SERVER`); fill the real `kFlag0/1/2` in that branch (len
  `<= kSlotSize - 2`). Flash that one badge.
- **Everything else** ships as `Client` with dummy flags, so reversing a
  player's own badge reveals no real challenge content.

## xboard protocol

- Recv ids added before `MAX` in `Logic/XBoardRecvFn.h`: `VAULT_HELLO_ID`,
  `VAULT_BTN_FWD_ID`, `VAULT_NAME_REQ_ID`, `VAULT_NAME_RESP_ID`.
- Packets (`VaultCommon.h`): `VaultBtnPacket { u16 button; }`,
  `VaultNamePacket { char name[kVaultNameBufLen]; }` (`kVaultNameBufLen =
  NAME_LEN + 1 = 23`). Hello and name-req carry a single ignored marker byte.
- The wire buffer size (`kVaultNameBufLen`) is deliberately independent of the
  pool slot size (`kSlotSize`).
