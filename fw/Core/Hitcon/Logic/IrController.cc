#include "IrController.h"

#include <App/HardwareTestApp.h>
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/Display/display.h>
#include <Logic/GameLogic.h>
#include <Logic/IrController.h>
#include <Logic/PreparedData.h>
#include <Logic/RandomPool.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <stdlib.h>

#include <cstring>

using namespace hitcon::service::sched;
using hitcon::game::gameLogic;
using hitcon::game::kDataSize;
using hitcon::game::kInternalGenChance;
using hitcon::game::kInternalGenMinQueueAvailable;

namespace hitcon {
namespace ir {

namespace {

static char SURPRISE_NAME[] = "You got pwned!";

}  // anonymous namespace

IrController irController;

IrController::IrController()
    : routine_task(950, (callback_t)&IrController::RoutineTask, this, 1000),
      broadcast_task(800, (callback_t)&IrController::BroadcastIr, this),
      send2game_task(800, (callback_t)&IrController::Send2Game, this),
      showtext_task(800, (callback_t)&IrController::ShowText, this),
      send_lock(true), recv_lock(true), disable_broadcast(false),
      received_packet_cnt(0), priority_data_len_(0) {}

void IrController::Send2Game(void* arg) {
  GamePacket* game = reinterpret_cast<GamePacket*>(arg);
  gameLogic.AcceptData(game->col, game->data);
  send_lock = true;
}
void IrController::ShowText(void* arg) {
  struct ShowPacket* pkt = reinterpret_cast<struct ShowPacket*>(arg);
  badge_controller.SetStoredApp(badge_controller.GetCurrentApp());
  show_name_app.SetSurpriseMsg(pkt->message);
  show_name_app.SetMode(Surprise);
  badge_controller.change_app(&show_name_app);
}

void IrController::Init() {
  irLogic.SetOnPacketReceived((callback_t)&IrController::OnPacketReceived,
                              this);
  badge_controller.SetCallback((callback_t)&IrController::SendShowPacket, this,
                               SURPRISE_NAME);
  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void IrController::OnPacketReceived(void* arg) {
  received_packet_cnt++;

  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);
  IrData* data = reinterpret_cast<IrData*>(&packet->data_[1]);

  // Game
  if (data->type == packet_type::kGame) {
    if (send_lock) {
      send_lock = false;
      scheduler.Queue(&send2game_task, &data->game);
    }
  } else if (data->type == packet_type::kTest) {
    hardware_test_app.CheckIr(&data->show);
  } else if (data->type == packet_type::kShow) {
    scheduler.Queue(&showtext_task, &data->show);
  } else if (data->type == packet_type::kPartition) {
    g_prepared_data.SetPartitionAndShow(data->partition.partition);
  }
}

int IrController::prob_f(int lf) { return v[0] * lf * lf + v[1] * lf + v[2]; }

void IrController::RoutineTask(void* unused) {
  if (gameLogic.IsGameReady()) {
    // Update parameters from load factor.
    int lf = irLogic.GetLoadFactor();

    // Determine if we want to send a packet.
    {
      uint32_t prob_max = prob_f(100);
      uint32_t rand_num = g_fast_random_pool.GetRandom() % prob_max;
      if (rand_num > prob_f(lf) && send_lock) {
        send_lock = false;
        scheduler.Queue(&broadcast_task, nullptr);
      }
    }

    // Determine if we want to internally generate.
    {
      uint32_t rand_num = g_fast_random_pool.GetRandom() % 65536;
      if (rand_num < kInternalGenChance &&
          gameLogic.GetAcceptDataQueueAvailable() >
              kInternalGenMinQueueAvailable) {
        // Generate random.
        gameLogic.DoRandomData();
      }
    }

    TrySendPriority();
  }
}

void IrController::BroadcastIr(void* unused) {
  if (disable_broadcast) return;

  if (!TrySendPriority()) return;

  uint8_t cell_data[kDataSize];
  int col = g_fast_random_pool.GetRandom() % hitcon::game::kNumCols;
  g_prepared_data.GetRandomDataForIrTransmission(cell_data, &col);
  IrData irdata = {
      .ttl = 0,
      .type = packet_type::kGame,
      .game =
          {
              .col = col,
          },
  };
  memcpy(irdata.game.data, cell_data, kDataSize);
  uint8_t irdata_len = 12;
  irLogic.SendPacket(reinterpret_cast<uint8_t*>(&irdata), irdata_len);

  send_lock = true;
}

void IrController::SendShowPacket(char* msg) {
  IrData irdata = {
      .ttl = 0,
      .type = packet_type::kShow,
  };
  size_t length = strlen(msg);
  memcpy(irdata.show.message, msg, length);
  memcpy(&priority_data_, &irdata, sizeof(irdata));
  priority_data_len_ = sizeof(priority_data_) / sizeof(uint8_t);
  ;
  TrySendPriority();
}

bool IrController::TrySendPriority() {
  if (priority_data_len_ == 0) return true;

  uint8_t irdata_len = sizeof(priority_data_) / sizeof(uint8_t);
  bool ret = irLogic.SendPacket(reinterpret_cast<uint8_t*>(&priority_data_),
                                irdata_len);
  if (ret) {
    priority_data_len_ = 0;
  }
  return false;
}

}  // namespace ir
}  // namespace hitcon
