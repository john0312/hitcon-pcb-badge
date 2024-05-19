#ifndef HITCON_LOGIC_MENU_TREE_H_
#define HITCON_LOGIC_MENU_TREE_H_

#include <stdint.h>
#include <Util/callback.h>
#include <Service/ButtonService.h>

namespace hitcon {

// This is where the primary logic of the menu tree is implemented.
class MenuTree final {
 public:
  MenuTree();

  // This is called whenever a button is pressed,
  // usually by the ButtonService.
  void OnButton(button_t button);

  // This is called whenever a packet is received from cross board
  // connector, usually by CrossBoardService.
  void OnCrossBoardPacket(uint8_t* packet);

  // This is called by UsbService whenever we're plugged into a
  // computer.
  void OnUsbInsertion();
};

}  // namespace hitcon

#endif  // #ifndef HITCON_LOGIC_MENU_TREE_H_
