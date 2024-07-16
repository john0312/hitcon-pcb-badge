#include <IrController.h>
#include <game.h>
#include <stddef.h>
#include <stdint.h>

namespace hitcon {

/*Definition of IR content.*/
struct IrData {
  uint8_t type;
  uint8_t data[GAME_DATA_SIZE];
  score_t score;
};

class IrController {
 public:
  IrData SendIr2Game(IrPacket &packet);
  void InitBroadcastService(uint8_t game_types);
 private:
  IrPacket BroadcastIr(IrData ir_data);   
}

}