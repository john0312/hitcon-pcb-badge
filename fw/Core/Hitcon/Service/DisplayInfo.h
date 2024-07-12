#ifndef SERVICE_DISLAY_INFO_H_
#define SERVICE_DISLAY_INFO_H_
#include <stddef.h>
namespace hitcon {

namespace {

constexpr size_t DISPLAY_FRAME_SIZE = 16; // 8 bit/row x 16 row = 16 bytes
constexpr size_t DISPLAY_FRAME_BATCH = 2;
constexpr size_t DISPLAY_UPDATE_PERIOD = 4; // must be 2^n where n is integer
}  // namespace

}  // namespace hitcon

#endif  // #ifndef SERVICE_DISLAY_INFO_H_
