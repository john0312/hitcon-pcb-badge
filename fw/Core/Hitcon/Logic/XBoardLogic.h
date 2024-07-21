#ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
#define HITCON_LOGIC_XBOARD_LOGIC_H_

#include <Util/callback.h>
#include <stddef.h>
#include <stdint.h>

#include "Service/Sched/Scheduler.h"
#include "Service/XBoardService.h"
#include "usart.h"

namespace hitcon {
namespace service {
namespace xboard {

struct PacketCallbackArg {
	uint8_t *data;
	uint8_t len;
};

enum UsartConnectState { Init, Connect, Disconnect };

constexpr size_t MAX_XBOARD_PACKET_LEN = 32;
constexpr size_t RX_BUF_SZ = 512;

void send_ping();

class XBoardLogic {
   public:
    XBoardLogic();

    void Init();

    // Call QueuePacketForTx() to queue a packet for transmission on
    // the XBoard connection.
    void QueuePacketForTx(uint8_t* packet, size_t packet_len);

    // On detected connection from a remote board, this will be called.
    void SetOnConnectCallback(callback_t callback, void* callback_arg1);

    // On detected disconnection from a remote board, this will be called.
    void SetOnDisconnectCallback(callback_t callback, void* callback_arg1);

    // On received a packet from a remote board, this will be called with a
    // pointer to packet struct.
    void SetOnPacketCallback(callback_t callback, void *self/*, PacketCallbackArg *callback_arg*/);

    void Routine(void*);

   private:
    hitcon::service::sched::PeriodicTask _routine_task;
    // uint8_t rx_buf[MAX_XBOARD_PACKET_LEN] = {0};
    // size_t prod_head = 0;
    // size_t cons_head = 0;
    callback_t packet_arrive_handler = nullptr;
    void *packet_arrive_handler_self = nullptr;

    UsartConnectState connect_state = UsartConnectState::Init;
    callback_t disconnect_handler = nullptr;
    void *disconnect_handler_self = nullptr;
    callback_t connect_handler = nullptr;
    void *connect_handler_self = nullptr;

    void OnByteArrive(void *);
    void ParsePacket();
    void CheckPing();
};

extern XBoardLogic g_xboard_logic;

}  // namespace xboard
}  // namespace service
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
