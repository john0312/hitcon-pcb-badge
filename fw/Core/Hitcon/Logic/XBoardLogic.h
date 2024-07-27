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

constexpr size_t RX_BUF_SZ = 512;

void send_ping();

class XBoardLogic {
   public:
    XBoardLogic();

    void Init();

    // Encapsulate data into a packet and then put into the tx queue.
    void QueueDataForTx(uint8_t* data, size_t data_len);

    // On detected connection from a remote board, this will be called.
    void SetOnConnect(callback_t callback, void* callback_arg1);

    // On detected disconnection from a remote board, this will be called.
    void SetOnDisconnect(callback_t callback, void* callback_arg1);

    // On received a packet from a remote board, this will be called with a
    // pointer to packet struct.
    void SetOnPacketArrive(callback_t callback, void *self);

    void Routine(void*);

   private:
    hitcon::service::sched::PeriodicTask _routine_task;
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
