#ifndef LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
#define LOGIC_XBOARD_GAME_CONTROLLER_DOT_H

namespace hitcon {

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
  // How many calls to GetRandomDataForXBoardTransmission() do we need?
  int random_send_left_;

  // The current index we're sending for SendAllData().
  int send_all_col_;
  int send_all_row_;

  // Tell the other side how long our queue is.
  void SendFlowControlSignal();

  // Periodically called.
  void Routine();
};

}  // namespace hitcon

#endif  // LOGIC_XBOARD_GAME_CONTROLLER_DOT_H
