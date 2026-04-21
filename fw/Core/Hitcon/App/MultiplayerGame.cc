#include <App/MultiplayerGame.h>
#include <Logic/GameController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardLogic.h>

#include <cstring>

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
      GameOver();
      break;
    case PACKET_ATTACK:
      RecvAttackPacket(packet);
      break;
  }
}

void MultiplayerGame::SendGameOver() {
  GameOverPacket packet = {
      .packetType = XboardPacketType::PACKET_GAME_OVER,
      .nonce = savedNonce = (uint16_t)g_fast_random_pool.GetRandom(),
      .score = GetScore()};
  hitcon::SetBufferToBadgeId(packet.username);
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
  hitcon::SetBufferToBadgeId(packet.username);
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
  g_xboard_logic.SetOnPacketArrive((callback_t)&MultiplayerGame::OnXboardRecv,
                                   this, GetXboardRecvId());
#pragma GCC diagnostic pop
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
