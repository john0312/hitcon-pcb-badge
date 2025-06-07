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

constexpr size_t PACKET_QUEUE_SIZE = 4;

enum PacketStatus : uint8_t {
  kFree,
  kWaitSignStart,
  kWaitSignDone,
  kWaitTransmit
};

constexpr size_t MAX_PACKET_DATA_SIZE = 13;

struct SignedPacket {
  PacketStatus status;
  packet_type type;
  size_t signatureOffset;
  uint8_t data[MAX_PACKET_DATA_SIZE];
  size_t dataSize;
  uint8_t sig[ECC_SIGNATURE_SIZE];
};

class GameController {
 public:
  GameController();

  void Init();
  /**
   * TODO: figure out the function signature.
   */
  bool SignAndSendData(packet_type packetType, const uint8_t *data,
                       size_t size);

 private:
  /*
  Internal state of the game controller.
  0 - Not initialized.
  1 - Initialized. After Init().
  2 - Waiting for hash processor to compute private key.
  3 - Got private key. Announce the pubkey.
  */
  int state_;

  uint8_t
      privkey_src_[PerBoardData::kSecretLen + sizeof(PRIVATE_KEY_SRC_PREFIX)];

  hitcon::service::sched::PeriodicTask routine_task;

  bool TrySendPubAnnounce();
  void OnPrivKeyHashFinish(void *arg2);
  void RoutineFunc();

  size_t signingPacketId;
  SignedPacket packet_queue_[PACKET_QUEUE_SIZE];
  void OnPacketSignFinish(hitcon::ecc::Signature *signature);
};
enum GameType : uint8_t { kNone, kSnake, kTetris };

}  // namespace game

extern hitcon::game::GameController g_game_controller;

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_GAME_CONTROLLER_H_
