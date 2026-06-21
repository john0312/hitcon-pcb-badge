#include <App/NeverGonna.h>
#include <App/app.h>
#include <Logic/Display/display.h>

namespace hitcon {
namespace never {

Gonna gonna;

volatile int rules = 0x1bf52;
// Own section so the 12-byte initializer stays contiguous and in order: the
// provisioner finds and patches this placeholder by its byte pattern.
// The `.data.` prefix is load-bearing: it makes the linker's *(.data*) place
// the section within _sdata.._edata, so startup copies the patched flash value
// into RAM. A plain section name lands past _edata, is never copied, and reads
// garbage at runtime.
__attribute__((section(".data.placeholder_never_other_guy"))) union {
  char desert_you[12];
  int hurt_you[3];
} other_guy = {.desert_you = "@@so_do_i@@"};

void Gonna::OnEntry() {
  //   GiveYouUp();
  //   LetYouDown();
  //   RunAroundAndDesertYou();
  //   display_set_mode_scroll_text(understand);
}
void Gonna::OnExit() {}
void Gonna::OnButton(button_t button) {}

void Gonna::GiveYouUp() {
  for (int i = 0, d = rules; i < 6; ++i, d /= 10) {
    switch (i) {
      case 0:
        understand[i] = (d % 10) << 4;
        understand[1] = (d % 10) << 4;
        break;
      case 1:
        understand[1] += d % 10;
        understand[0] += d % 10;
        break;
      case 2:
        understand[0] += d % 10;
        understand[3] += d % 10;
        break;
      case 3:
        understand[2] += (d % 10) << 4;
        understand[i] += (d % 10) << 4 | (d % 10);
        break;
      case 4:
        understand[2] += d % 10;
        understand[3] -= d % 10;
        break;
      case 5:
        understand[3] -= d % 10;
        break;
    }
  }
  understand[1] += rules / 10000;
  understand[4] = '{';
}

void Gonna::LetYouDown() {
  uint32_t strangers[3];
  strangers[0] = HAL_GetUIDw0();
  strangers[1] = HAL_GetUIDw1();
  strangers[2] = HAL_GetUIDw2();
  strangers[0] ^= other_guy.hurt_you[0];
  strangers[1] ^= other_guy.hurt_you[1];
  strangers[2] ^= other_guy.hurt_you[2];
  memcpy(understand + 5, strangers, sizeof(strangers));
}

void Gonna::RunAroundAndDesertYou() { understand[17] = '}'; }

namespace {

// constructors live in .init_array, which the linker script KEEPs, so Init()
// and other functions are guaranteed to survive into the binary.
// `used` stops the compiler from optimizing away this function.
__attribute__((constructor, used)) void I_just_wanna_tell_you() {
  // Storing each address through a volatile makes the reference an observable
  // side effect the compiler must not optimize away, keeping the symbol alive.
  void (Gonna::*volatile k0)() = &Gonna::GiveYouUp;
  void (Gonna::*volatile k1)() = &Gonna::LetYouDown;
  void (Gonna::*volatile k2)() = &Gonna::RunAroundAndDesertYou;

  // suppress unused-variable warnings
  (void)k0;
  (void)k1;
  (void)k2;
}

}  // namespace

}  // namespace never
}  // namespace hitcon
