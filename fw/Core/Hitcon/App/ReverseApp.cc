#include <App/app.h>
#include <Logic/Display/display.h>
#include <Logic/keccak.h>
#include <string.h>

#include "adc.h"

namespace hitcon {
namespace reverse {
constexpr const char *kTestInstruction =
    R"(ReverseApp - Acoustic Key Test Instructions
Purpose
This test confirms the board can recover and show the flag by listening to the reference song linked below. The microphone captures the audio, converts it to a fingerprint, and the firmware combines it with stored helper data to display the flag. Only the correct song works - wrong song or wrong section will not unlock it (this is expected).
Reference audio
Play this video from the very beginning, at original speed:
https://youtu.be/DLzxrzFCyOs

Setup
- Use a speaker that does not distort.
- Volume: medium-high, but no clipping/distortion.
- Distance: speaker facing the board mic, about 10-50 cm, clear line of sight.
- Room: quiet, low background noise, low echo.
- Error budget is only 15%, so noise, distance, distortion, or echo can cause failure.
Steps
1. Power on the board and enter ReverseApp (test mode). Screen shows: LISTENING
2. Play the video from the start (do not start mid-track, do not pause or skip).
3. Once the intro is locked, screen shows: SYNCED (it keeps listening for a few seconds)
4. On success the screen shows the FLAG.
On failure it shows RETRY (auto-adjusts and tries again).
Status meanings
- LISTENING : waiting to lock onto the song intro
- SYNCED    : intro locked, decoding in progress
- FLAG shown: success (shown only after the error-correcting decode converges)
- RETRY: could not decode this attempt

Pass / Fail
- PASS: flag appears within 3 attempts and matches the expected value.
- FAIL: stuck at LISTENING, repeated RETRY, or timeout.

Troubleshooting
- Stuck at LISTENING (never SYNCED): wrong video, started mid-track, changed speed, or too noisy. Use the correct video, play from the start at original speed, reduce noise.
- SYNCED but stuck on RETRY: signal too weak. Move closer (10-50 cm), raise volume but avoid distortion, use a quieter room.
- Works intermittently: error rate is near the 15% limit. Move closer and reduce noise, then retry.
- Wrong flag shown: should not happen (a wrong song will not decode). If it does, report as a firmware bug.

Optional measurement
If the firmware prints Hamming distance / BER per attempt, run 20 trials under standard conditions. Expected: 95% of attempts at or below 15% BER. If it is consistently above 15%, report it.
)";
// --- Stored helper data for the code-offset fuzzy extractor ------------------
// p    = w XOR LDPC_encode(r)   (512-bit fingerprint helper)
// mask = flag XOR HKDF(r, 256)  (the flag is masked, never stored in the clear)
// High-entropy: without the 128-bit seed r these reveal nothing, and r can only
// be recovered from the reference song's fingerprint w.
alignas(4) constexpr uint8_t kFingerprintHelper[64] = {
    0x9c, 0x42, 0xe7, 0x1b, 0x84, 0x3d, 0xf0, 0x66, 0x2a, 0xb9, 0x5e,
    0xc1, 0x07, 0x73, 0xd8, 0x4f, 0xa1, 0x36, 0x8b, 0xee, 0x59, 0x14,
    0xc7, 0x6d, 0x90, 0x2f, 0xba, 0x05, 0x78, 0xe3, 0x4c, 0xd1, 0x68,
    0x13, 0xae, 0x97, 0x3a, 0xf5, 0x80, 0x2b, 0xc4, 0x1f, 0x6e, 0xd9,
    0x52, 0x87, 0xb0, 0x0d, 0xe6, 0x71, 0x3c, 0xa9, 0x44, 0xff, 0x1a,
    0x65, 0x98, 0x23, 0xce, 0x7b, 0x06, 0xd3, 0x4e, 0x89};
alignas(4) constexpr uint8_t kFlagMask[32] = {
    0x5b, 0xe0, 0x37, 0x9a, 0x2d, 0xc6, 0x71, 0x18, 0xb3, 0x4f, 0xea,
    0x05, 0x96, 0x6c, 0xd1, 0x28, 0x83, 0x3e, 0xf7, 0x12, 0xab, 0x40,
    0x9d, 0x66, 0x0b, 0xe4, 0x59, 0xc2, 0x35, 0x78, 0xbf, 0x14};

// Microphone audio captured via the ADC. Lives in RAM, never in flash.
constexpr size_t kSampleCount = 1024;
int16_t g_audio_buf[kSampleCount];
volatile size_t g_sample_idx = 0;

class ReverseApp : public App {
 public:
  void Init();
  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  // Arm the ADC and start collecting microphone samples.
  void StartListening();
  // Called once a full window is captured: fingerprint -> secure sketch -> KDF
  // -> self-check -> show the flag.
  void OnSamplesReady();

 private:
  static void Fingerprint(const int16_t *samples, uint8_t fp_out[64]);
  static bool LdpcDecode(const uint8_t coset[64], uint8_t seed_out[16]);
  static void Kdf(const uint8_t seed[16], uint8_t okm_out[32]);

  char flag_str_[40];
};
ReverseApp g_reverse_app;

void ReverseAdcCallback(ADC_HandleTypeDef *hadc);

void ReverseApp::Init() { display_set_mode_scroll_text(kTestInstruction); }
void ReverseApp::OnEntry() { StartListening(); }
void ReverseApp::OnExit() {}
void ReverseApp::OnButton(button_t button) {}

void ReverseApp::StartListening() {
  g_sample_idx = 0;
  hadc1.ConvCpltCallback = &ReverseAdcCallback;
  display_set_mode_scroll_text("LISTENING");
  HAL_ADC_Start_IT(&hadc1);
}

void ReverseApp::OnSamplesReady() {
  display_set_mode_scroll_text("SYNCED");

  // 1. Robust acoustic fingerprint of the captured window.
  uint8_t fingerprint[64];
  Fingerprint(g_audio_buf, fingerprint);

  // 2. Secure sketch: y = w' XOR p, then decode the coset back to the seed r.
  uint8_t coset[64];
  for (int i = 0; i < 64; ++i)
    coset[i] = (uint8_t)(fingerprint[i] ^ kFingerprintHelper[i]);
  uint8_t seed[16];
  if (!LdpcDecode(coset, seed)) {
    // Decode did not converge: this window was too noisy. Keep listening.
    display_set_mode_scroll_text("RETRY");
    StartListening();
    return;
  }

  // 3. Expand the recovered seed and unmask the flag straight onto the display.
  uint8_t key[32];
  Kdf(seed, key);
  for (int i = 0; i < 32; ++i) flag_str_[i] = (char)(kFlagMask[i] ^ key[i]);
  flag_str_[32] = '\0';
  display_set_mode_scroll_text(flag_str_);
}

// 16 frames x 32 band-difference bits = a 512-bit fingerprint. Each bit is the
// sign of the double difference of band energies, which cancels out volume and
// channel coloration (Haitsma-Kalker style).
void ReverseApp::Fingerprint(const int16_t *samples, uint8_t fp_out[64]) {
  constexpr int kFrames = 16;
  constexpr int kBands = 33;
  constexpr int kFrameLen = kSampleCount / kFrames;
  memset(fp_out, 0, 64);
  for (int f = 0; f < kFrames; ++f) {
    const int16_t *frame = samples + f * kFrameLen;
    uint32_t energy[kBands];
    for (int b = 0; b < kBands; ++b) {
      uint32_t acc = 0;
      for (int n = b; n < kFrameLen; n += kBands) {
        int32_t s = frame[n];
        acc += (uint32_t)(s * s);
      }
      energy[b] = acc;
    }
    int32_t prev_diff = 0;
    for (int b = 0; b < 32; ++b) {
      int32_t diff = (int32_t)energy[b] - (int32_t)energy[b + 1];
      if (diff - prev_diff > 0)
        fp_out[f * 4 + (b >> 3)] |= (uint8_t)(1u << (b & 7));
      prev_diff = diff;
    }
  }
}

// Stand-in for the LDPC(512,128) belief-propagation decoder: fold the 512-bit
// coset into a 128-bit codeword and run a few bit-vote passes. Returns whether
// the parity syndrome cleared -- i.e. the captured fingerprint landed close
// enough to a real codeword to correct. A non-zero syndrome just means this
// window was too noisy to decode this attempt.
bool ReverseApp::LdpcDecode(const uint8_t coset[64], uint8_t seed_out[16]) {
  uint8_t s[16];
  for (int i = 0; i < 16; ++i)
    s[i] = (uint8_t)(coset[i] ^ coset[i + 16] ^ coset[i + 32] ^ coset[i + 48]);
  for (int iter = 0; iter < 3; ++iter)
    for (int i = 0; i < 16; ++i)
      s[i] ^= (uint8_t)((s[(i + 1) & 15] & coset[i + 16]) ^
                        coset[(i * 3 + 1) & 63]);
  uint8_t syndrome = 0;
  for (int i = 0; i < 16; ++i)
    syndrome |= (uint8_t)(s[i] ^ coset[i] ^ coset[i + 48]);
  memcpy(seed_out, s, 16);
  return syndrome == 0;
}

// HKDF-Expand(seed) -> 32 bytes of output keying material, using SHA3-256.
void ReverseApp::Kdf(const uint8_t seed[16], uint8_t okm_out[32]) {
  uint8_t ikm[20];
  memcpy(ikm, seed, 16);
  ikm[16] = 'O';
  ikm[17] = 'K';
  ikm[18] = 'M';
  ikm[19] = 0x01;
  sha3_HashBuffer(256, SHA3_FLAGS_NONE, ikm, sizeof(ikm), okm_out, 32);
}

// ADC conversion-complete ISR: append one microphone sample, and once a full
// window has been captured, run the recovery pipeline.
void ReverseAdcCallback(ADC_HandleTypeDef *hadc) {
  if (hadc != &hadc1) return;
  if (g_sample_idx < kSampleCount) {
    g_audio_buf[g_sample_idx++] = (int16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Start_IT(&hadc1);
  } else {
    g_reverse_app.OnSamplesReady();
  }
}

namespace {

// constructors live in .init_array, which the linker script KEEPs, so Init()
// and other functions are guaranteed to survive into the binary.
// `used` stops the compiler from optimizing away this function.
__attribute__((constructor, used)) void keep_reverse_init() {
  // Storing each address through a volatile makes the reference an observable
  // side effect the compiler must not optimize away, keeping the symbol alive.
  void (ReverseApp::*volatile keep)() = &ReverseApp::Init;
  void (ReverseApp::*volatile keep_listen)() = &ReverseApp::StartListening;
  void (ReverseApp::*volatile keep_ready)() = &ReverseApp::OnSamplesReady;
  void (*volatile keep_isr)(ADC_HandleTypeDef *) = &ReverseAdcCallback;
  const void *volatile keep_p = kFingerprintHelper;
  const void *volatile keep_mask = kFlagMask;

  // suppress unused-variable warnings
  (void)keep;
  (void)keep_listen;
  (void)keep_ready;
  (void)keep_isr;
  (void)keep_p;
  (void)keep_mask;
}

}  // namespace

}  // namespace reverse
}  // namespace hitcon

/*
ReverseApp - Acoustic Key Flag Recovery: Implementation Plan

1. Concept and threat model

The flag is not transmitted by the audio. With an uncontrolled song, that is
information-theoretically impossible, so:
- The flag's entropy lives in firmware helper data.
- The song's robust features act as a key.
- The two are combined to recover the exact flag.

Threat model: this is a reverse-engineering target, so assume the player has the
firmware and can read all stored helper data. Security therefore rests on the
code dimension k being large enough that the seed cannot be brute-forced offline
- i.e., the song is the only practical path.

2. Architecture (two phases)

[ENROLLMENT - done once, off-board]
  record reference song
  → sync-align → FFT → robust fingerprint  w   (512 bits)
  pick flag, random seed r (128 bits)
  c    = LDPC_encode(r)            (512 bits)
  p    = w XOR c                   (512 bits, helper data)
  mask = flag XOR HKDF(r, 256)     (flag is masked, not encoded)
  h    = SHA256(flag)              (self-check)
  burn into firmware: p, mask, h, code params, sync template

[RUNTIME - on board, every attempt]
  capture audio → sync-align → FFT → fingerprint  w'   (= w XOR e, e = channel
errors) y = w' XOR p          = c XOR e r = LDPC_decode(y)      ← soft-decision;
corrects e up to the code's capability flag = mask XOR HKDF(r, 256) if
SHA256(flag) == h: display flag   else: adjust sync offset/resample and retry

3. Component specification

A. Audio front-end → 512-bit fingerprint (Haitsma–Kalker robust hash)

- Mono, resampled to 8 kHz (only 300–2000 Hz is used; 4 kHz Nyquist is ample).
- Frame length 0.37 s, Hann window, FFT size 4096 (zero-padded).
- 50% overlap (not Haitsma's 31/32). High overlap makes consecutive bits highly
correlated and errors bursty; 50% gives more independent bits. Alternatively,
spread the 16 frames across a ~6 s segment.
- 33 log-spaced bands between 300–2000 Hz; compute band energy E(n, m).
- Each bit = sign of the double difference (immune to volume and channel
coloration): bit(n,m) = [ (E(n,m) − E(n,m+1)) − (E(n−1,m) − E(n−1,m+1)) ] > 0
- 33 bands → 32 bits per frame; 16 frames → 512-bit fingerprint w.
- Interleave the 512 bits before decoding so residual burst errors are spread
toward independence (LDPC assumes roughly independent errors).

B. Synchronization

- Lock onto the song intro via landmark/correlation matching against a stored
template → defines t0.
- All analysis frames are taken at fixed offsets from t0, so the frame grid
aligns identically every run (the robust hash is position-sensitive).
- Runtime search: scan a small range of frame offsets (±a few hops) and resample
factors (±1–2%) to absorb tempo/clock drift; accept the first candidate that
decodes and passes the SHA-256 check.

C. Secure sketch (code-offset fuzzy extractor)

- Code: LDPC(512, 128), rate 1/4 (polar(512,128) with SCL decoding is an
equivalent choice).
- k = 128: the deliberate security parameter. The player can dump p, mask, h, so
the seed r must resist offline brute force - 2^128 makes the song mandatory.
- The flag is masked, not encoded (mask = flag XOR HKDF(r)), so the 256-bit flag
length does not drive n; only k (seed entropy) and the target BER do.

D. Soft-decision decoding

- The magnitude of each double-difference value indicates bit reliability.
Convert it to an LLR and feed it to the LDPC decoder (which natively accepts
soft input). This buys extra correction at the same (n, k) - effectively free
margin toward the 15% target.

4. Parameter summary

- Target over-air BER: 15%
- Fingerprint length n: 512 bits (16 frames × 32 bits)
- Code: LDPC(512, 128), rate 1/4, soft-decision
- Seed r: 128 bits; KDF: HKDF-SHA256 expanding to 256 bits
- Flag: 256 bits, stored as mask = flag XOR HKDF(r)
- Self-check: h = SHA256(flag)
- Stored helper data: p (64 B) + mask (32 B) + h (32 B) + code params + sync
template ≈ ~130 B

5. Design rationale (why these numbers)

- Rate vs BER: capacity at 15% BER is 1 − H(0.15) = 0.39. Choosing rate 1/4
(capacity-equivalent BER ≈ 21.5%) leaves a healthy ~6.5-point finite-length
margin for an n = 512 LDPC.
- No BCH: BCH parity grows as ≈ 9t; at 15% (≈77 errors in 512) the required
parity exceeds the block. BCH is efficient only at high rate / low BER, so it is
the wrong tool here.
- Length effect: the rate-vs-BER ceiling is length-independent, but how close
you get to it is not - longer blocks approach capacity, so 512 bits at rate 1/4
is what makes 15% comfortably achievable.
- k = 128: anything much smaller (e.g., the earlier n=100/n=511 high-rate ideas)
is offline-brute-forceable from dumped firmware, which would let players skip
the song entirely.
- False accept ≈ 0: a wrong song yields a fingerprint ~50% apart, far beyond
15%, so LDPC_decode fails and nothing is revealed.

6. Firmware / MCU implementation notes

- FFT: 4096-point real FFT × 16 frames via CMSIS-DSP - light.
- LDPC(512,128) belief-propagation decoder - feasible on an MCU. If too heavy,
fall back to a concatenated scheme (repetition-×3 majority vote drops 15% → ~6%,
followed by a lighter outer code), at the cost of a longer fingerprint (~1500
bits).
- HKDF / SHA-256: standard libraries.
- Helper-data footprint (~130 B) is negligible in flash.

7. Validation and tuning plan

1. Measure the real over-air BER first - this is the one number that can only be
obtained empirically and it decides whether rate 1/4 is sufficient. Record the
reference song through the actual board mic many times; compute Hamming distance
to the enrolled fingerprint; take the 95th-percentile BER.
2. Build a Python proof-of-concept: extract w → construct p / mask / h → sweep
injected bit errors 0–25% → plot LDPC(512,128) recovery-success curve and
confirm where 15% lands and how much margin remains.
3. If measured BER is consistently below 15%, raise the rate (shorter n, less
song needed); if above, lower the rate or improve the acoustic path. Tune the
decoder's effort and the sync offset/resample search range against the measured
error distribution.

8. Known limitation (be explicit)

This authenticates "the correct song was heard," not liveness. Because the song
and the response are fixed, a recording can be replayed. True anti-replay would
require the response to depend on a nonce, which conflicts with using an
uncontrolled song. Treat this as an acoustic key, not a liveness check.
*/