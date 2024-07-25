#include <Logic/IrController.h>
#include <Logic/game.h>
#include <Service/IrService.h>
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
      send_lock(true), recv_lock(true) {}

void IrController::Send2Game(void* arg) {
  GamePacket* game = reinterpret_cast<GamePacket*>(arg);
  game_accept_data(game->col, game->data);
  send_lock = true;
}

void IrController::Init() {
  irLogic.SetOnPacketReceived((callback_t)&IrController::OnPacketReceived,
                              this);
  // TODO: Remove the srand and time
  srand(time(NULL));
}

void IrController::OnPacketReceived(void* arg) {
  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);
  IrData* data = reinterpret_cast<IrData*>(packet->data_);

  // Game
  if (data->packet_type == 0) {
    if (send_lock) {
      send_lock = false;
      scheduler.Queue(&send2game_task, &data->game);
    }
  }
}

int IrController::prob_f(int lf) { return v[0] * lf * lf + v[1] * lf + v[2]; }

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
  for (int i = 0; i < GAME_DATA_SIZE; ++i) {
    // Get data and send to packet
  }
  send_lock = true;
}

}  // namespace ir
}  // namespace hitcon
