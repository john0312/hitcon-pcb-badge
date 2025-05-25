#include "IrController.h"

#include <App/HardwareTestApp.h>
#include <App/ShowNameApp.h>
#include <Logic/BadgeController.h>
#include <Logic/BaseStnHub.h>
#include <Logic/Display/display.h>
#include <Logic/IrController.h>
#include <Logic/RandomPool.h>
#include <Service/IrService.h>
#include <Service/Sched/Scheduler.h>
#include <stdlib.h>

#include <cstring>

using namespace hitcon::service::sched;

namespace hitcon {
namespace ir {

namespace {

static char SURPRISE_NAME[] = "You got pwned!";

}  // anonymous namespace

IrController irController;

IrController::IrController()
    : routine_task(950, (callback_t)&IrController::RoutineTask, this, 1000),
      broadcast_task(800, (callback_t)&IrController::BroadcastIr, this),
      showtext_task(800, (callback_t)&IrController::ShowText, this),
      send_lock(true), recv_lock(true), disable_broadcast(false),
      received_packet_cnt(0), priority_data_len_(0) {}

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
  switch (data->type) {
    case packet_type::kGame:
      // removed
      break;
    case packet_type::kTest:
      hardware_test_app.CheckIr(&data->show);
      break;
    case packet_type::kShow:
      scheduler.Queue(&showtext_task, &data->show);
      break;
  }

  // forward all packets to the base station hub
  hitcon::basestn::g_basestn_hub.OnIrPacketRecv(
      reinterpret_cast<uint8_t*>(data), packet->size_);
}

int IrController::prob_f(int lf) { return v[0] * lf * lf + v[1] * lf + v[2]; }

void IrController::RoutineTask(void* unused) {
  // remove generating random number
}

void IrController::BroadcastIr(void* unused) {
  if (disable_broadcast) return;

  if (!TrySendPriority()) return;

  // remove broadcasting
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
