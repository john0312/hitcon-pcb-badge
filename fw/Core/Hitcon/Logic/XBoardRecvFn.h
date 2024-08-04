#ifndef HITCON_SERVICE_XBOARD_RECV_FN_H_
#define HITCON_SERVICE_XBOARD_RECV_FN_H_

namespace hitcon {
namespace service {
namespace xboard {

// Different functions should be bound to different IDs
// define your function id before max
enum RecvFnId {
    // MAX is to express the length of callback function array
    MAX
};

}
}  // namespace service
}  // namespace hitcon
#endif
