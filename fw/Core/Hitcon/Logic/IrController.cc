#include "Logic/IrController.h"

namespace {

IrData IrController::SendIr2Game(IrPacket &packet) {
  /*if (packet.size() != sizeof(IrData)) {
     Error & return;
  }*/
  IrData ir_data = static_cast<IrData>(packet.data());
  return ir_data;
}

IrController::InitBroadcastService(uint8_t game_types) {
  for (int i = 0; i < game_types; ++i){
    BroadcastIr()
  }
}

IrPacket IrController::BroadcastIr(IrData &ir_data){
  // TODO
}

}