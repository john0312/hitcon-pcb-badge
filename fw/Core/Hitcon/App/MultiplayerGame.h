#ifndef MULTIPLAYER_GAME_H
#define MULTIPLAYER_GAME_H

#include <Logic/GameController.h>
#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>

#include "app.h"

namespace hitcon {

namespace app {

namespace multiplayer {

enum XboardPacketType : uint8_t {
  PACKET_START = 1,
  PACKET_ABORT,
  PACKET_GAME_OVER,
  PACKET_GAME_OVER_ACK,
  PACKET_ATTACK
};

enum PlayerCount { SINGLEPLAYER = 1, MULTIPLAYER };

struct GameOverPacket {
  XboardPacketType packetType;
  uint8_t username[hitcon::ir::IR_USERNAME_LEN];
  uint16_t nonce;
  uint16_t score;
} __attribute__((packed));

static_assert(sizeof(GameOverPacket) ==
              sizeof(XboardPacketType) + sizeof(uint16_t) +
                  hitcon::ir::IR_USERNAME_LEN + sizeof(uint16_t));

class MultiplayerGame : public App {
 private:
  uint16_t savedNonce;
  PlayerCount playerCount;
  void OnXboardRecv(void *arg);
  void SendGameOverAck(hitcon::service::xboard::PacketCallbackArg *rcvdPacket);

 protected:
  void UploadSingleplayerScore();
  void UploadMultiplayerScore(
      hitcon::service::xboard::PacketCallbackArg *packet);
  void SendGameOver();
  void SendStartGame();
  void SendAbortGame();
  void SendAttack(uint8_t atk);
  bool IsMultiplayer() const;
  virtual void GameEntry() = 0;
  virtual void GameExit() = 0;
  virtual void StartGame() = 0;
  virtual void AbortGame() = 0;
  virtual void GameOver() = 0;
  virtual hitcon::service::xboard::RecvFnId GetXboardRecvId() const = 0;
  virtual hitcon::game::EventType GetGameType() const = 0;
  virtual uint32_t GetScore() const = 0;
  virtual void RecvAttackPacket(
      hitcon::service::xboard::PacketCallbackArg *packet) = 0;

 public:
  void SetPlayerCount(PlayerCount playerCount);
  void OnEntry() override final;
  void OnExit() override final;
};

}  // namespace multiplayer

}  // namespace app

}  // namespace hitcon

#endif  // MULTIPLAYER_GAME_H