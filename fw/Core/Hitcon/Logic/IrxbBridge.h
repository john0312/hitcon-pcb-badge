#ifndef HITCON_LOGIC_IRXB_BRIDGE_H_
#define HITCON_LOGIC_IRXB_BRIDGE_H_

#include <Logic/EcLogic.h>
#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>
#include <Service/Sched/DelayedTask.h>

namespace hitcon {
namespace ir_xb_bridge {

enum TamaState {
  kTamaStateInit = 0,
  kTamaStateWaitSignStart,
  kTamastateWaitSignDone,
  kTamaStateWaitSend,
  kTamaStateWaitRestore,
  kTamaStateDone,
};

enum ScoreState {
  kScoreStateInit = 0,
  kScoreStateWaitSendGetScore,
  kScoreStateWaitSetScore,
  kScoreStateDone,
};

}  // namespace ir_xb_bridge

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
  void TamaRoutine();
  void ScoreRoutine();
  void OnPacketReceived(void* arg);
  void EnsureRoutineQueued();

  void OnTamaSignDone(hitcon::ecc::Signature* signature);

  hitcon::service::sched::DelayedTask routine_task_;

  hitcon::ir_xb_bridge::TamaState tama_state_;
  hitcon::ir_xb_bridge::ScoreState score_state_;

  hitcon::ir::IrData tama_data_;
  hitcon::ir::IrData score_data_;
  // TODO: add transmission buffer score and tama
  // TODO: add a "last received" timestamp for score and tama
  int state_;

  int tx_cnt_;
  int rx_cnt_;

  char disp_txt_[4];

  int show_cycles_;

  bool routine_queued_ = false;
};

extern IrxbBridge g_irxb_bridge;

}  // namespace hitcon

#endif  // HITCON_LOGIC_IRXB_BRIDGE_H_