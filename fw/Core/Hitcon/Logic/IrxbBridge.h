#ifndef HITCON_LOGIC_IRXB_BRIDGE_H_
#define HITCON_LOGIC_IRXB_BRIDGE_H_

#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/DelayedTask.h>

namespace hitcon {

class IrxbBridge {
 public:
  IrxbBridge();

  void Init();

  // Called by BadgeController
  void OnXBoardBasestnConnect();
  void OnXBoardBasestnDisconnect();

 private:
  void RoutineTask();
  void OnPacketReceived(void* arg);

  hitcon::service::sched::DelayedTask routine_task_;
  int state_;

  // Buffer for constructing an IrPacket from XBoard payload.
  ir::IrPacket received_packet_buffer_;
};

extern IrxbBridge g_irxb_bridge;

}  // namespace hitcon

#endif  // HITCON_LOGIC_IRXB_BRIDGE_H_