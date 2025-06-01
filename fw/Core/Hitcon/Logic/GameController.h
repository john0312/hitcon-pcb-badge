#ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
#define HITCON_LOGIC_GAME_CONTROLLER_H_

#include <Logic/IrController.h>
#include <Service/PerBoardData.h>
#include <Service/Sched/Scheduler.h>
#include <stdint.h>

namespace hitcon {
namespace game {

constexpr uint8_t PRIVATE_KEY_SRC_PREFIX[] = {'2', '0', '2', '5',
                                              'P', 'R', 'I', 'V'};

enum EventType : uint8_t {
  kNone = 0,
  kSnake = 1,
  kTetris = 2,
  kDino = 3,
  kShake = 16
};

struct SingleBadgeActivity {
  EventType eventType;
  uint8_t eventData[3];
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

class GameController {
 public:
  GameController();

  void Init();

  bool SendTwoBadgeActivity(const TwoBadgeActivity &data);
  bool SendProximity(const Proximity &data);
  bool SendSingleBadgeActivity(const SingleBadgeActivity &data);

  void NotifyPubkeyAck();

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

  uint8_t
      privkey_src_[PerBoardData::kSecretLen + sizeof(PRIVATE_KEY_SRC_PREFIX)];

  hitcon::service::sched::PeriodicTask routine_task;

  bool TrySendPubAnnounce();
  void OnPrivKeyHashFinish(void *arg2);
  void RoutineFunc();
  /**
   * Copy the username into the specified buffer. Buffer should be at least
   * IR_USERNAME_LEN in size. This function does not perform any size checks!
   * Caller is expected to do so.
   */
  void GetUsername(uint8_t *buf);
};

}  // namespace game

extern hitcon::game::GameController g_game_controller;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
