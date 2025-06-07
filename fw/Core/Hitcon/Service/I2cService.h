#ifndef SERVICE_I2C_SERVICE_H_
#define SERVICE_I2C_SERVICE_H_

namespace hitcon {

enum {
  I2C_STATE_IDLE,
  I2C_STATE_WAITING_ACK,
  I2C_STATE_START,
  I2C_STATE_STOP,
  I2C_MEM_READ,
  I2C_MEM_WRITE
};

// only one slave address
class I2cService {
 private:
  int _state;
  uint8_t _address;

 public:
  void WriteReg();
  void ReadReg();
};

}  // namespace hitcon

#endif