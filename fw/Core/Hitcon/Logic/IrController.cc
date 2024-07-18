#include <Logic/IrController.h>

namespace {

// constexpr int IrAllowedBroadcastCol[] = {...};

}

namespace hitcon {
namespace ir {

IrController::IrController()
    : routine_task(950, (callback_t)&IrController::RoutineTask, this, 1000) {}

void IrController::SendIr2Game(IrPacket& packet) {
  /*if (packet.size() != sizeof(IrData)) {
     Error & return;
  }*/
  IrData* ir_data = reinterpret_cast<IrData*>(packet.data());
  // TODO: Send ir_data to game.cc
}

void IrController::Init() {
  irLogic.SetOnPacketReceived((callback_t)&IrController::OnPacketReceived,
                              this);
}

void IrController::OnPacketReceived(void* arg) {
  IrPacket* packet = reinterpret_cast<IrPacket*>(arg);

  // Handle this packet.
}

void IrController::RoutineTask(void* unused) {
  // Update parameters from load factor.
  // int lf = irLogic.GetLoadFactor();
  // param_xxx = f(lf);

  // Determine if we want to send a packet.
  // int rand_num = ...
  // if (rand_num < param_xxx) {
  // We want to send a packet.
  // int col = rand...

  // irLogic.SendPacket(..., ...);
  //}
}

void IrController::InitBroadcastService(uint8_t game_types) {
  for (int i = 0; i < game_types; ++i) {
    // BroadcastIr()
  }
}

}  // namespace ir
}  // namespace hitcon
