#include <App/MultiplayerGame.h>
#include <Logic/GameController.h>
#include <Logic/IrController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>

#include <cstring>

using hitcon::game::EventType;
using hitcon::game::SingleBadgeActivity;
using hitcon::game::TwoBadgeActivity;
using hitcon::service::xboard::g_xboard_logic;
using hitcon::service::xboard::PacketCallbackArg;

namespace hitcon {

namespace app {

namespace multiplayer {

void MultiplayerGame::OnXboardRecv(void *arg) {
  PacketCallbackArg *packet = reinterpret_cast<PacketCallbackArg *>(arg);
  switch (packet->data[0]) {
    case PACKET_START:
      StartGame();
      break;
    case PACKET_ABORT:
      AbortGame();
      break;
    case PACKET_GAME_OVER:
      SendGameOverAck(packet);
      GameOver();
      break;
    case PACKET_GAME_OVER_ACK:
      UploadMultiplayerScore(packet);
      GameOver();
      break;
    case PACKET_ATTACK:
      RecvAttackPacket(packet);
      break;
  }
}

void MultiplayerGame::UploadMultiplayerScore(PacketCallbackArg *packet) {
  if (packet->len != sizeof(GameOverPacket)) return;
  GameOverPacket *gameOverPacket =
      reinterpret_cast<GameOverPacket *>(packet->data);
  if (gameOverPacket->packetType != PACKET_GAME_OVER &&
      gameOverPacket->packetType != PACKET_GAME_OVER_ACK)
    return;
  if (gameOverPacket->nonce != savedNonce) return;
  TwoBadgeActivity activity = {.gameType = GetGameType(),
                               .myScore = GetScore(),
                               .otherScore = gameOverPacket->score,
                               .nonce = gameOverPacket->nonce};
  memcpy(activity.otherUser, gameOverPacket->username,
         sizeof(activity.otherUser));
  g_game_controller.SendTwoBadgeActivity(activity);
}

void MultiplayerGame::UploadSingleplayerScore() {
  SingleBadgeActivity activity = {
      .eventType = GetGameType(), .myScore = 0, .nonce = 0};
  uint32_t score = GetScore();
  if (score >= 1024) {
    score = 1023;
  }
  activity.myScore = score;
  activity.nonce = (uint16_t)g_fast_random_pool.GetRandom();
  g_game_controller.SendSingleBadgeActivity(activity);
}

void MultiplayerGame::SendGameOver() {
  GameOverPacket packet = {
      .packetType = XboardPacketType::PACKET_GAME_OVER,
      .nonce = savedNonce = (uint16_t)g_fast_random_pool.GetRandom(),
      .score = GetScore()};
  g_game_controller.GetUsername(packet.username);
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t *>(&packet),
                                sizeof(packet), GetXboardRecvId());
}

void MultiplayerGame::SendGameOverAck(PacketCallbackArg *rcvdPacket) {
  if (rcvdPacket->len != sizeof(GameOverPacket)) return;
  GameOverPacket *_rcvdPacket =
      reinterpret_cast<GameOverPacket *>(rcvdPacket->data);
  if (_rcvdPacket->packetType != PACKET_GAME_OVER) return;
  GameOverPacket packet = {.packetType = XboardPacketType::PACKET_GAME_OVER_ACK,
                           .nonce = savedNonce = _rcvdPacket->nonce,
                           .score = GetScore()};
  g_game_controller.GetUsername(packet.username);
  g_xboard_logic.QueueDataForTx(reinterpret_cast<uint8_t *>(&packet),
                                sizeof(packet), GetXboardRecvId());
}

void MultiplayerGame::SendStartGame() {
  XboardPacketType packetType = PACKET_START;
  g_xboard_logic.QueueDataForTx((uint8_t *)&packetType, sizeof(packetType),
                                GetXboardRecvId());
}

void MultiplayerGame::SendAbortGame() {
  XboardPacketType packetType = PACKET_ABORT;
  g_xboard_logic.QueueDataForTx((uint8_t *)&packetType, sizeof(packetType),
                                GetXboardRecvId());
}

void MultiplayerGame::SendAttack(uint8_t atk) {
  uint8_t data[2] = {XboardPacketType::PACKET_ATTACK, atk};
  g_xboard_logic.QueueDataForTx(data, sizeof(data), GetXboardRecvId());
}

void MultiplayerGame::OnEntry() {
  GameEntry();
  g_xboard_logic.SetOnPacketArrive((callback_t)&MultiplayerGame::OnXboardRecv,
                                   this, GetXboardRecvId());
}

void MultiplayerGame::OnExit() { GameExit(); }

void MultiplayerGame::SetPlayerCount(PlayerCount playerCount) {
  this->playerCount = playerCount;
}

bool MultiplayerGame::IsMultiplayer() const {
  return playerCount == PlayerCount::MULTIPLAYER;
}

}  // namespace multiplayer

}  // namespace app

}  // namespace hitcon
