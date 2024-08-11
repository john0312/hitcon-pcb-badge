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
    : _send_routine(500, (task_callback_t)&XBoardGameController::SendRoutine,
                    this, 500),
      accept_data_task(
          500, (task_callback_t)&XBoardGameController::AcceptDataTask, this) {}

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
  random_send_left_ = hitcon::game::kNumCols * 100 / percentage;
  send_state = WaitAck;
}

void XBoardGameController::SendOneData() {
  if (send_state != Idle) return;
  GetData();
}

void XBoardGameController::SendAllData() {
  // TODO
}

// private functions

bool XBoardGameController::IsBusy() { return send_state != Idle; }

void XBoardGameController::SendRoutine() {
  switch (send_state) {
    case Idle:
      break;
    case PrepareData:
      GetData();
      break;
    case WaitAck:
      if (++wait_count > 3) {
        wait_count = 0;
        Queue2XBoard();
      }
      break;
    case Acked:
      if (--random_send_left_ > 0) {
        GetData();
      } else {
        send_state = Idle;
      }
      break;
  }
}

void XBoardGameController::Queue2XBoard() {
  xboard::g_xboard_logic.QueueDataForTx(
      reinterpret_cast<uint8_t*>(&to_send), game::kDataSize,
      xboard::RecvFnId::XBOARD_GAME_CONTROLLER);
}

void XBoardGameController::GetData() {
  send_state = PrepareData;
  int col = g_fast_random_pool.GetRandom() % game::kNumCols;
  to_send.col = col;
  bool ok = game::gameLogic.GetRandomDataForIrTransmission(to_send.data, &col);
  if (ok) {
    Queue2XBoard();
    send_state = WaitAck;
  }
}

void XBoardGameController::RecvAck() {
  if (send_state == WaitAck) {
    send_state = Acked;
  }
}

void XBoardGameController::AcceptDataTask() {
  while (recv_len > 0) {
    GamePacket* game = &recv_buffer[recv_cons];
    game::gameLogic.AcceptData(game->col, game->data);
    recv_cons = (recv_cons + 1) % RECV_BUFFER_SIZE;
    --recv_len;
  }
}

void XBoardGameController::RemoteRecv(xboard::PacketCallbackArg* pkt) {
  if (recv_len == RECV_BUFFER_SIZE) {
    // drop data
    return;
  }
  memcpy(reinterpret_cast<void*>(&recv_buffer[recv_prod]), pkt->data, pkt->len);
  scheduler.Queue(&accept_data_task, nullptr);
  recv_prod = (recv_prod + 1) % RECV_BUFFER_SIZE;
  ++recv_len;
  xboard::g_xboard_logic.QueueDataForTx(
      nullptr, 0, xboard::RecvFnId::XBOARD_GAME_CONTROLLER_ACK);
}

}  // namespace xboard_game_controller
}  // namespace hitcon
