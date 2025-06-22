#ifndef HITCON_LOGIC_CDC_LOGIC_FN_H_
#define HITCON_LOGIC_CDC_LOGIC_FN_H_
namespace hitcon {
namespace logic {
namespace cdc {
// Different functions should be bound to different IDs
// define your function id before max
enum FnId {
  PLACE_HOLDER,
  QueueStationTX,
  QueueXBoardTX = 0x11,
  // MAX is to express the length of callback function array
  MAX
};
}  // namespace cdc
}  // namespace logic
}  // namespace hitcon
#endif  // HITCON_LOGIC_CDC_LOGIC_FN_H_
