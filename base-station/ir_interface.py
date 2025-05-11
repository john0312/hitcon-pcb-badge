

"""
USB Serial Commands:
PacketSize - 1 byte. If 0, hardware side should respond with another 0, this is for sync-ing.
PacketSequence - 1 byte. A random byte to denote which response maps to which request.
    The response should have the same packet sequence as the request.
PacketType - 1 byte
    * 0x01 for QueueTxBufferRequest
    * 0x81 for QueueTxBufferResponse
    * 0x02 for RetrieveRxBufferRequest
    * 0x82 for RetrieveRxBufferResponse
    * 0x03 for GetStatusRequest
    * 0x83 for GetStatusResponse
    * 0x04 for SendStatusRequest
    * 0x84 for SendStatusResponse

PacketData - x bytes

QueueTxBufferRequest: (Computer to Badge)
x bytes - The entire buffer data to queue for transmit.

QueueTxBufferResponse: (Badge to Computer)
1 byte - 0x01 for buffer full, 0x02 for success.

RetrieveRxBufferRequest: (Computer to Badge)
None

RetrieveRxBufferResponse: (Badge to Computer)
1 byte - 0x01 for buffer empty, 0x02 for success.
x bytes for the entire buffer received.

GetStatusRequest: (Computer to Badge)
None

GetStatusResponse: (Badge to Computer)
1 byte - 0x00 for failure, 0x01 for success.
1 byte - Status:
            * 0x01: Set for ready to transmit, a transmit buffer is empty and available.
            * 0x02: Set for ready to receive, a received buffer has been populated with received packet.

SendStatusRequest: (Badge to Computer)
1 byte - The status, same as above.

SendStatusResponse: (Computer to Badge)
None

"""

class IrInterface:
    def __init__(self):
        pass

    async def trigger_send_packet(self, data: bytes) -> bool:
        # Triggers the ir interface to send a packet.
        # Returns True if sent successfully, False otherwise.
        pass

    async def get_next_packet(self) -> bytes:
        # Wait until the next packet arrives, then return its raw data.
        pass
