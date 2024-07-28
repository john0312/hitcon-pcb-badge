#include "IrController.h"
#include <Logic/IrController.h>
#include <Logic/RandomPool.h>
#include <Logic/game.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <stdlib.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::game::game_accept_data;
using hitcon::game::IrAllowBroadcastCnt;
using hitcon::game::IrAllowedBroadcastCol;
using hitcon::game::kDataSize;

namespace hitcon {
namespace ir {

IrController irController;

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
  
  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void IrController::OnPacketReceived(void* arg) {
  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);
  IrData* data = reinterpret_cast<IrData*>(packet->data_);

  // Game
  if (data->type == packet_type::kGame) {
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
  uint32_t prob_max = prob_f(100);
  uint32_t rand_num = g_fast_random_pool.GetRandom() % prob_max;
  if (rand_num > prob_f(lf) && send_lock) {
    send_lock = false;
    scheduler.Queue(&broadcast_task, nullptr);
  }
}

void IrController::BroadcastIr(void* unused) {
  uint8_t cell_data[kDataSize];
  int col = g_fast_random_pool.GetRandom() % hitcon::game::kNumCols;
  bool ok =
      hitcon::game::get_random_cell_data_for_ir_transmission(cell_data, &col);
  if (ok) {
    IrData irdata = {
        .ttl = 0,
        .type = packet_type::kGame,
        .game =
            {
                .col = col,
            },
    };
    memcpy(irdata.game.data, cell_data, kDataSize);
    uint8_t irdata_len = sizeof(irdata) / sizeof(uint8_t);
    irLogic.SendPacket(reinterpret_cast<uint8_t*>(&irdata), irdata_len);
  }

  send_lock = true;
}

}  // namespace ir
}  // namespace hitcon
