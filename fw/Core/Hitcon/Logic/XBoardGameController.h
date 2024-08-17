#ifndef LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
#define LOGIC_XBOARD_GAME_CONTROLLER_DOT_H

#include <Logic/IrController.h>
#include <Logic/XBoardLogic.h>

namespace hitcon {

namespace xboard_game_controller {
enum SendState { Idle, Sending, WaitAck, Acked };

/*Definition of IR content.*/
struct AckPacket {
  // Number successfully received packet.
  uint8_t finished;
  // How many remaining slots left?
  uint16_t available;
};

class XBoardGameController {
 public:
  XBoardGameController();

  void Init();

  // Send percentage of our data to the other side.
  void SendPartialData(int percentage = 25);

  // Send all data without flow control.
  void SendAllData();

  // Return true if busy.
  bool IsBusy();

  void OnConnect();
  void OnDisconnect();

  int GetRemainingDataCount() { return random_send_left_; }

 private:
  hitcon::service::sched::PeriodicTask _send_routine;
  SendState send_state = Idle;
  uint16_t wait_count = 0;

  // How many calls to GetRandomDataForXBoardTransmission() do we need?
  int random_send_left_;

  // The current index we're sending for SendAllData().
  int send_all_col_;
  int send_all_row_;

  // How many slot does the remote have available for receiving data?
  int remote_buffer_left_;

  // How man times has SendRoutine() ran?
  int current_cycle_;

  // Is remote there?
  bool connected_;

  // Debugging statistics.
  int data_from_remote_cnt_;
  int successfully_received_;
  int ack_timeout_;
  int malformed_gamepkt_;
  int malformed_ackpkt_;

  // Tell the other side how long our queue is.
  void SendFlowControlSignal();

  // Periodically called.
  void SendRoutine();

  // Sends one data with GetRandomDataForXBoardTransmission().
  void SendOneData();

  // Callback for receiving AckPacket.
  void RecvAck(hitcon::service::xboard::PacketCallbackArg* opkt);

  // Send AckPacket to remote.
  void SendAck(uint8_t finished);

  // remote side
  void RemoteRecv(hitcon::service::xboard::PacketCallbackArg* pkt);

  void TryExitApp();
};

extern XBoardGameController g_xboard_game_controller;

}  // namespace xboard_game_controller

}  // namespace hitcon

#endif  // LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
