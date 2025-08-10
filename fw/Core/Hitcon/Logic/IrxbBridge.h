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
  bool RoutineInternal();
  void OnPacketReceived(void* arg);

  hitcon::service::sched::DelayedTask routine_task_;
  int state_;

  int tx_cnt_;
  int rx_cnt_;

  char disp_txt_[4];

  int show_cycles_;
};

extern IrxbBridge g_irxb_bridge;

}  // namespace hitcon

#endif  // HITCON_LOGIC_IRXB_BRIDGE_H_