
#include <App/ConnectScreenApp.h>
#include <Logic/BaseStnHub.h>

using namespace hitcon::app::connect_screen;
using namespace hitcon::basestn;

namespace hitcon {
namespace service {
namespace xboard {

// register external calls for connect/disconnect events here
void OnConnectHandler() { g_basestn_hub.OnXBoardConnect(); }
void OnDisconnectHandler() {}

}  // namespace xboard
}  // namespace service
}  // namespace hitcon
