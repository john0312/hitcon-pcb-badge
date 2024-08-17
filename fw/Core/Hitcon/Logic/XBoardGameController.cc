#include <App/ConnectMenuApp.h>
#include <App/SendDataApp.h>
#include <Logic/BadgeController.h>
#include <Logic/GameLogic.h>
#include <Logic/IrController.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardGameController.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>

#include <cstring>

namespace game = hitcon::game;
namespace xboard = hitcon::service::xboard;
using namespace hitcon::service::sched;
using namespace hitcon::ir;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

namespace hitcon {
namespace xboard_game_controller {

XBoardGameController g_xboard_game_controller;

// what should the periodic of _send_routine be?
XBoardGameController::XBoardGameController()
    : _send_routine(650, (task_callback_t)&XBoardGameController::SendRoutine,
                    this, 50),
      send_state(Idle), current_cycle_(0), successfully_received_(0),
      ack_timeout_(0), malformed_gamepkt_(0), malformed_ackpkt_(0) {}

void XBoardGameController::Init() {
  xboard::g_xboard_logic.SetOnPacketArrive(
      (callback_t)&XBoardGameController::RemoteRecv, this,
      xboard::RecvFnId::XBOARD_GAME_CONTROLLER);
  xboard::g_xboard_logic.SetOnPacketArrive(
      (callback_t)&XBoardGameController::RecvAck, this,
      xboard::RecvFnId::XBOARD_GAME_CONTROLLER_ACK);
  scheduler.Queue(&_send_routine, nullptr);
  scheduler.EnablePeriodic(&_send_routine);
}

void XBoardGameController::SendPartialData(int percentage) {
  random_send_left_ += hitcon::game::kNumRows *
                       hitcon::game::XBoardAllowedBroadcastColCnt * 100 /
                       percentage;
  if (send_state == Idle) {
    send_state = Sending;
  }
}

void XBoardGameController::SendOneData() {
  // QueueDataForTx copies the data on invocation, so we don't need to ensure
  // it's life cycle survives the entire transmit process.
  hitcon::ir::GamePacket to_send;
  int col;
  game::gameLogic.GetRandomDataForXBoardTransmission(to_send.data, &col);
  to_send.col = static_cast<uint8_t>(col);
  xboard::g_xboard_logic.QueueDataForTx(
      reinterpret_cast<uint8_t*>(&to_send), sizeof(to_send),
      xboard::RecvFnId::XBOARD_GAME_CONTROLLER);
  if (remote_buffer_left_ > 0) remote_buffer_left_--;
}

void XBoardGameController::SendAllData() {
  // TODO
}

// private functions

bool XBoardGameController::IsBusy() { return send_state != Idle; }

void XBoardGameController::SendRoutine() {
  current_cycle_++;
  if (!connected_) return;

  if (current_cycle_ % 8 == 0) {
    SendAck(0);
  }

  switch (send_state) {
    case Idle:
      break;
    case Sending:
      if (remote_buffer_left_ > 0) {
        SendOneData();
        send_state = WaitAck;
      }
      break;
    case WaitAck:
      if (++wait_count > 5) {
        wait_count = 0;
        send_state = Acked;
        ack_timeout_++;
      }
      break;
    case Acked:
      random_send_left_--;
      if (random_send_left_ > 0) {
        send_state = Sending;
      } else {
        send_state = Idle;
        TryExitApp();
      }
      break;
  }
}

void XBoardGameController::RecvAck(xboard::PacketCallbackArg* opkt) {
  if (opkt->len != sizeof(AckPacket)) {
    malformed_ackpkt_++;
    return;
  }
  AckPacket* pkt = reinterpret_cast<AckPacket*>(opkt);
  remote_buffer_left_ = pkt->available - 2;
  if (remote_buffer_left_ < 0) remote_buffer_left_ = 0;
  // Cap the maximum so we don't send too many packets in one go.
  if (remote_buffer_left_ > 3) remote_buffer_left_ = 3;

  if (pkt->finished != 0) {
    if (send_state == WaitAck) {
      // Remote successfully received the packet.
      successfully_received_++;
      send_state = Acked;
    }
  }
}

void XBoardGameController::RemoteRecv(xboard::PacketCallbackArg* pkt) {
  if (pkt->len != sizeof(GamePacket)) {
    malformed_gamepkt_++;
    return;
  }
  GamePacket* gpkt = reinterpret_cast<GamePacket*>(pkt->data);
  bool ret = game::gameLogic.AcceptData(gpkt->col, gpkt->data);
  data_from_remote_cnt_++;
  SendAck(ret ? 1 : 0);
}

void XBoardGameController::SendAck(uint8_t finished) {
  AckPacket pkt;
  pkt.finished = finished;
  pkt.available = static_cast<uint16_t>(
      hitcon::game::gameLogic.GetAcceptDataQueueAvailable());
  xboard::g_xboard_logic.QueueDataForTx(
      reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt),
      xboard::RecvFnId::XBOARD_GAME_CONTROLLER_ACK);
}

void XBoardGameController::TryExitApp() {
  if (badge_controller.GetCurrentApp() == &g_send_data_app) {
    if (connected_) {
      badge_controller.change_app(&connect_menu);
    } else {
      badge_controller.BackToMenu(&g_send_data_app);
    }
  }
}

void XBoardGameController::OnConnect() {
  connected_ = true;
  random_send_left_ = 0;
  remote_buffer_left_ = 0;
  send_state = Idle;
}

void XBoardGameController::OnDisconnect() {
  connected_ = false;
  random_send_left_ = 0;
  remote_buffer_left_ = 0;
  send_state = Idle;
  TryExitApp();
}

}  // namespace xboard_game_controller
}  // namespace hitcon
