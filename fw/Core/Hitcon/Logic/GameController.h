#ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
#define HITCON_LOGIC_GAME_CONTROLLER_H_

#include <Logic/IrController.h>
#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

namespace hitcon {
namespace game {

enum EventType : uint8_t {
  kNone = 0,
  kSnake = 1,
  kTetris = 2,
  kDino = 3,
  kTama = 4,
  kShake = 16
};

struct SingleBadgeActivity {
  EventType eventType;
  uint16_t myScore;
  uint16_t nonce;
};

struct TwoBadgeActivity {
  EventType gameType;
  uint8_t otherUser[hitcon::ir::IR_USERNAME_LEN];
  uint16_t myScore;
  uint16_t otherScore;
  uint16_t nonce;
};

struct Proximity {
  uint8_t power;
  uint16_t nonce;
};

constexpr size_t kPubAnnounceCycleInterval = 484;  // 1.49s * 484 ~= 12 minutes

class GameController {
 public:
  GameController();

  void Init();

  bool SendTwoBadgeActivity(const TwoBadgeActivity &data);
  bool SendProximity(const Proximity &data);
  bool SendSingleBadgeActivity(const SingleBadgeActivity &data);

  void NotifyPubkeyAck();
  /**
   * Returns a pointer to a buffer that holds the username, buffer holds
   * IR_USERNAME_LEN in size.
   *
   * Note that the returned buffer may no longer be valid after the current
   * task ends.
   */
  const uint8_t *GetUsername();

  /**
   * Copy the username into the specified buffer. Buffer should be at least
   * IR_USERNAME_LEN in size. This function does not perform any size checks!
   * Caller is expected to do so.
   */
  bool SetBufferToUsername(uint8_t *ptr);

 private:
  /*
  Internal state of the game controller.
  0 - Not initialized.
  1 - Initialized. After Init().
  2 - Waiting for hash processor to compute private key.
  3 - Got private key. Announce the pubkey.
  4 - Sent public key announce packet.
  5 - Got public key acknowledgement.
  */
  int state_;

  int pubAnnounceCnt;
  hitcon::service::sched::PeriodicTask pubAnnounceTask;
  bool pubAnnonceEnabled = false;

  void TrySendPubAnnounce();
};

}  // namespace game

extern hitcon::game::GameController g_game_controller;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
