#include <Logic/GameLogic.h>
#include <Logic/RandomPool.h>
#include <Logic/XBoardGameController.h>
#include <Logic/XBoardLogic.h>
#include <Logic/XBoardRecvFn.h>
#include <Service/Sched/Scheduler.h>
#include <Service/Sched/Task.h>

namespace game = hitcon::game;
namespace xboard = hitcon::service::xboard;
using namespace hitcon::service::sched;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

namespace hitcon {
namespace xboard_game_controller {

XBoardGameController::XBoardGameController()
    : _send_routine(490, (task_callback_t)&XBoardGameController::SendRoutine,
                    this, 50) {}

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
      to_send, game::kDataSize, xboard::RecvFnId::XBOARD_GAME_CONTROLLER);
}

void XBoardGameController::GetData() {
  send_state = PrepareData;
  int col = g_fast_random_pool.GetRandom() % game::kNumCols;
  bool ok = game::gameLogic.GetRandomDataForIrTransmission(to_send, &col);
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

void XBoardGameController::RemoteRecv(xboard::PacketCallbackArg* pkt) {
  xboard::g_xboard_logic.QueueDataForTx(
      nullptr, 0, xboard::RecvFnId::XBOARD_GAME_CONTROLLER_ACK);
}

}  // namespace xboard_game_controller
}  // namespace hitcon
