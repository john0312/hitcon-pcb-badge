#ifndef LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
#define LOGIC_XBOARD_GAME_CONTROLLER_DOT_H

#include <Logic/XBoardLogic.h>

namespace hitcon {

namespace xboard_game_controller {
enum SendState { Idle, PrepareData, WaitAck, Acked };

class XBoardGameController {
 public:
  XBoardGameController();

  void Init();

  // Send percentage of our data to the other side.
  void SendPartialData(int percentage = 25);

  // Sends one data with GetRandomDataForXBoardTransmission().
  void SendOneData();

  // Send all data without flow control.
  void SendAllData();

  // Return true if busy.
  bool IsBusy();

 private:
  hitcon::service::sched::PeriodicTask _send_routine;
  SendState send_state = Idle;
  uint8_t wait_count = 0;
  uint8_t to_send[game::kDataSize];

  // How many calls to GetRandomDataForXBoardTransmission() do we need?
  int random_send_left_;

  // The current index we're sending for SendAllData().
  int send_all_col_;
  int send_all_row_;

  // Tell the other side how long our queue is.
  void SendFlowControlSignal();

  // Periodically called.
  void SendRoutine();

  void Queue2XBoard();
  void GetData();
  void RecvAck();

  // remote side
  void RemoteRecv(hitcon::service::xboard::PacketCallbackArg* pkt);
};

}  // namespace xboard_game_controller

}  // namespace hitcon

#endif  // LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
