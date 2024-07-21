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

void uart_init();
void send_ping();
void uart_recv_cb(UART_HandleTypeDef* huart);

// class XBoardPacket {
//    public:
//     XBoardPacket() : size_(0){};

//     uint8_t* data() { return data_; }

//     size_t size() { return size_; }

//     void set_size(size_t s) { size_ = s; };

//    private:
//     size_t size_;
// };

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

    void OnByteArrive(void *);
};

extern XBoardLogic g_xboard_logic;

}  // namespace xboard
}  // namespace service
}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_XBOARD_LOGIC_H_
