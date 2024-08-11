#ifndef HITCON_SERVICE_XBOARD_RECV_FN_H_
#define HITCON_SERVICE_XBOARD_RECV_FN_H_

namespace hitcon {
namespace service {
namespace xboard {

// Different functions should be bound to different IDs
// define your function id before max
enum RecvFnId {
  SNAKE_RECV_ID,
  TEST_APP_RECV_ID,
  TETRIS_RECV_ID,
  XBOARD_GAME_CONTROLLER,
  XBOARD_GAME_CONTROLLER_ACK,
  // MAX is to express the length of callback function array
  MAX
};

}  // namespace xboard
}  // namespace service
}  // namespace hitcon
#endif
