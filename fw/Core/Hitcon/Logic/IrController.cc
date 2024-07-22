#include "IrController.h"
#include "game.h"
#include <stdlib.h>
#include <time.h>

using namespace hitcon::service::sched;
namespace hitcon {
namespace ir {

constexpr int IrAllowedBroadcastCol[] = {0, 2, 3, 4, 7};
constexpr int IrAllowBroadcastCnt = 5;

IrController::IrController()
    : routine_task(950, (callback_t)&IrController::RoutineTask, this, 1000),
	  broadcast_task(800, (callback_t)&IrController::BroadcastIr, this),
	  send2game_task(800, (callback_t)&IrController::Send2Game, this),
      recv_lock(true), send_lock(true)
    {
      srand(time(NULL));
      Init();
    }

void IrController::Send2Game(void* unused) {
  game_accept_data(callback_col, callback_data);
  send_lock = true;
}

void IrController::Init() {
  irLogic.SetOnPacketReceived((callback_t)&IrController::OnPacketReceived,
                              this);
  // Initialize v[0]
}


void IrController::OnPacketReceived(void* arg) {
  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);
  callback_col = packet->size;
  callback_data = packet->data;

  if (send_lock) {
    send_lock = false;
    scheduler.Queue(&send2game_task, nullptr);
  }
}

int IrController::prob_f(int lf) {
  return v[0] * lf * lf + v[1] * lf + v[2];
}

void IrController::RoutineTask(void* unused) {
  // Update parameters from load factor.
  int lf = irLogic.GetLoadFactor();

  // Determine if we want to send a packet.
  int rand_num = rand() % RAND_MAX;
  if (rand_num > prob_f(lf) && send_lock) {
    send_lock = false;
    scheduler.Queue(&broadcast_task, nullptr);
  }
}

void IrController::BroadcastIr(void* unused) {
  int type = rand() % IrAllowBroadcastCnt;
  send_lock = true;
}

}  // namespace ir
}  // namespace hitcon
